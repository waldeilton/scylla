/*
 * Copyright (C) 2015-present ScyllaDB
 */

/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#pragma once

#include <boost/test/unit_test.hpp>

#include "mutation.hh"

class mutation_partition_assertion {
    schema_ptr _schema;
    const mutation_partition& _m;
private:
    static mutation_partition compacted(const schema& s, const mutation_partition& m) {
        mutation_partition res(s, m);
        auto key = dht::decorate_key(s, partition_key::make_empty());
        res.compact_for_compaction(s, always_gc, key, gc_clock::time_point::min());
        return res;
    }
public:
    mutation_partition_assertion(schema_ptr s, const mutation_partition& m)
        : _schema(s)
        , _m(m)
    { }

    // If ck_ranges is passed, verifies only that information relevant for ck_ranges matches.
    mutation_partition_assertion& is_equal_to(const mutation_partition& other,
            const std::optional<query::clustering_row_ranges>& ck_ranges = {}) {
        return is_equal_to(*_schema, other, ck_ranges);
    }

    // If ck_ranges is passed, verifies only that information relevant for ck_ranges matches.
    mutation_partition_assertion& is_equal_to(const schema& s, const mutation_partition& other,
            const std::optional<query::clustering_row_ranges>& ck_ranges = {}) {
        if (ck_ranges) {
            mutation_partition_assertion(_schema, _m.sliced(*_schema, *ck_ranges))
                .is_equal_to(s, other.sliced(s, *ck_ranges));
            return *this;
        }
        if (!_m.equal(*_schema, other, s)) {
            BOOST_FAIL(format("Mutations differ, expected {}\n ...but got: {}",
                              mutation_partition::printer(s, other), mutation_partition::printer(*_schema, _m)));
        }
        if (!other.equal(s, _m, *_schema)) {
            BOOST_FAIL(format("Mutation inequality is not symmetric for {}\n ...and: {}",
                              mutation_partition::printer(s, other), mutation_partition::printer(*_schema, _m)));
        }
        return *this;
    }

    mutation_partition_assertion& is_equal_to_compacted(const schema& s,
                                                        const mutation_partition& other,
                                                        const std::optional<query::clustering_row_ranges>& ck_ranges = {}) {
        mutation_partition_assertion(s.shared_from_this(), compacted(s, _m))
            .is_equal_to(compacted(s, other), ck_ranges);
        return *this;
    }

    mutation_partition_assertion& is_equal_to_compacted(const mutation_partition& other,
                                                        const std::optional<query::clustering_row_ranges>& ck_ranges = {}) {
        return is_equal_to_compacted(*_schema, other, ck_ranges);
    }

    mutation_partition_assertion& is_not_equal_to(const mutation_partition& other) {
        return is_not_equal_to(*_schema, other);
    }

    mutation_partition_assertion& is_not_equal_to(const schema& s, const mutation_partition& other) {
        if (_m.equal(*_schema, other, s)) {
            BOOST_FAIL(format("Mutations equal but expected to differ: {}\n ...and: {}",
                              mutation_partition::printer(s, other), mutation_partition::printer(*_schema, _m)));
        }
        return *this;
    }

    mutation_partition_assertion& has_same_continuity(const mutation_partition& other) {
        if (!_m.equal_continuity(*_schema, other)) {
            BOOST_FAIL(format("Continuity doesn't match: {}\n ...and: {}", mutation_partition::printer(*_schema, other), mutation_partition::printer(*_schema, _m)));
        }
        return *this;
    }

    mutation_partition_assertion& is_continuous(const position_range& r, is_continuous cont = is_continuous::yes) {
        if (!_m.check_continuity(*_schema, r, cont)) {
            BOOST_FAIL(format("Expected range {} to be {} in {}", r, cont ? "continuous" : "discontinuous", mutation_partition::printer(*_schema, _m)));
        }
        return *this;
    }
};

static inline
mutation_partition_assertion assert_that(schema_ptr s, const mutation_partition& mp) {
    return {std::move(s), mp};
}

class mutation_assertion {
    mutation _m;
public:
    mutation_assertion(mutation m)
        : _m(std::move(m))
    { }

    // If ck_ranges is passed, verifies only that information relevant for ck_ranges matches.
    mutation_assertion& is_equal_to(const mutation& other, const std::optional<query::clustering_row_ranges>& ck_ranges = {}) {
        if (ck_ranges) {
            mutation_assertion(_m.sliced(*ck_ranges)).is_equal_to(other.sliced(*ck_ranges));
            return *this;
        }
        if (_m != other) {
            BOOST_FAIL(format("Mutations differ, expected {}\n ...but got: {}", other, _m));
        }
        if (other != _m) {
            BOOST_FAIL(format("Mutation inequality is not symmetric for {}\n ...and: {}", other, _m));
        }
        return *this;
    }

    mutation_assertion& is_equal_to_compacted(const mutation& other, const std::optional<query::clustering_row_ranges>& ck_ranges = {}) {
        mutation_assertion(_m.compacted()).is_equal_to(other.compacted(), ck_ranges);
        return *this;
    }

    mutation_assertion& is_not_equal_to(const mutation& other) {
        if (_m == other) {
            BOOST_FAIL(format("Mutations equal but expected to differ: {}\n ...and: {}", other, _m));
        }
        return *this;
    }

    mutation_assertion& has_schema(schema_ptr s) {
        if (_m.schema() != s) {
            BOOST_FAIL(format("Expected mutation of schema {}, but got {}", *s, *_m.schema()));
        }
        return *this;
    }

    mutation_assertion& has_same_continuity(const mutation& other) {
        assert_that(_m.schema(), _m.partition()).has_same_continuity(other.partition());
        return *this;
    }

    mutation_assertion& is_continuous(const position_range& r, is_continuous cont = is_continuous::yes) {
        assert_that(_m.schema(), _m.partition()).is_continuous(r, cont);
        return *this;
    }

    // Verifies that mutation data remains unchanged when upgraded to the new schema
    void is_upgrade_equivalent(schema_ptr new_schema) {
        mutation m2 = _m;
        m2.upgrade(new_schema);
        BOOST_REQUIRE(m2.schema() == new_schema);
        mutation_assertion(m2).is_equal_to(_m);

        mutation m3 = m2;
        m3.upgrade(_m.schema());
        BOOST_REQUIRE(m3.schema() == _m.schema());
        mutation_assertion(m3).is_equal_to(_m);
        mutation_assertion(m3).is_equal_to(m2);
    }
};

static inline
mutation_assertion assert_that(mutation m) {
    return { std::move(m) };
}

class mutation_opt_assertions {
    mutation_opt _mo;
public:
    mutation_opt_assertions(mutation_opt mo) : _mo(std::move(mo)) {}

    mutation_assertion has_mutation() {
        if (!_mo) {
            BOOST_FAIL("Expected engaged mutation_opt, but found not");
        }
        return { *_mo };
    }

    void has_no_mutation() {
        if (_mo) {
            BOOST_FAIL("Expected disengaged mutation_opt");
        }
    }
};

static inline
mutation_opt_assertions assert_that(mutation_opt mo) {
    return { std::move(mo) };
}
