/*
 * Copyright 2012-present ScyllaDB
 */

/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

verb [[with_client_info, with_timeout, one_way]] mutation (frozen_mutation fm, inet_address_vector_replica_set forward, gms::inet_address reply_to, unsigned shard, uint64_t response_id, std::optional<tracing::trace_info> trace_info [[version 1.3.0]], db::per_partition_rate_limit::info rate_limit_info [[version 5.1.0]]);
verb [[with_client_info, one_way]] mutation_done (unsigned shard, uint64_t response_id, db::view::update_backlog backlog [[version 3.1.0]]);
verb [[with_client_info, one_way]] mutation_failed (unsigned shard, uint64_t response_id, size_t num_failed, db::view::update_backlog backlog [[version 3.1.0]], replica::exception_variant exception [[version 5.1.0]]);
verb [[with_client_info, with_timeout]] counter_mutation (std::vector<frozen_mutation> fms, db::consistency_level cl, std::optional<tracing::trace_info> trace_info);
verb [[with_client_info, with_timeout, one_way]] hint_mutation (frozen_mutation fm, inet_address_vector_replica_set forward, gms::inet_address reply_to, unsigned shard, uint64_t response_id, std::optional<tracing::trace_info> trace_info [[version 1.3.0]] /* this verb was mistakenly introduced with optional trace_info */);
verb [[with_client_info, with_timeout]] read_data (query::read_command cmd, ::compat::wrapping_partition_range pr, query::digest_algorithm digest [[version 3.0.0]], db::per_partition_rate_limit::info rate_limit_info [[version 5.1.0]]) -> query::result [[lw_shared_ptr]], cache_temperature [[version 2.0.0]], replica::exception_variant [[version 5.1.0]];
verb [[with_client_info, with_timeout]] read_mutation_data (query::read_command cmd, ::compat::wrapping_partition_range pr) -> reconcilable_result [[lw_shared_ptr]], cache_temperature [[version 2.0.0]], replica::exception_variant [[version 5.1.0]];
verb [[with_client_info, with_timeout]] read_digest (query::read_command cmd, ::compat::wrapping_partition_range pr, query::digest_algorithm digest [[version 3.0.0]], db::per_partition_rate_limit::info rate_limit_info [[version 5.1.0]]) -> query::result_digest, api::timestamp_type [[version 1.2.0]], cache_temperature [[version 2.0.0]], replica::exception_variant [[version 5.1.0]];
verb [[with_timeout]] truncate (sstring, sstring);
verb [[with_client_info, with_timeout]] paxos_prepare (query::read_command cmd, partition_key key, utils::UUID ballot, bool only_digest, query::digest_algorithm da, std::optional<tracing::trace_info> trace_info) -> service::paxos::prepare_response [[unique_ptr]];
verb [[with_client_info, with_timeout]] paxos_accept (service::paxos::proposal proposal [[ref]], std::optional<tracing::trace_info> trace_info) -> bool;
verb [[with_client_info, with_timeout, one_way]] paxos_learn (service::paxos::proposal decision, inet_address_vector_replica_set forward, gms::inet_address reply_to, unsigned shard, uint64_t response_id, std::optional<tracing::trace_info>);
verb [[with_client_info, with_timeout, one_way]] paxos_prune (utils::UUID schema_id, partition_key key [[ref]], utils::UUID ballot, std::optional<tracing::trace_info> trace_info);
