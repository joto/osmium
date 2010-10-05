
#include "osmium.hpp"

namespace Osmium {

    namespace Handler {

            // if you change anything in this array, also change the corresponding struct in Statistics.hpp
            const char *Statistics::stat_names[] = {
                "nodes",
                "nodes_without_tags",
                "node_tags",
                "max_node_id",
                "max_tags_on_node",
                "ways",
                "way_tags",
                "way_nodes",
                "max_way_id",
                "max_tags_on_way",
                "max_nodes_on_way",
                "closed_ways",
                "relations",
                "relation_tags",
                "relation_members",
                "max_relation_id",
                "max_tags_on_relation",
                "max_members_on_relation",
                "users",
                "max_user_id",
                "anon_user_objects",
                "max_node_version",
                "max_way_version",
                "max_relation_version",
                "sum_node_version",
                "sum_way_version",
                "sum_relation_version",
                "max_changeset_id",
                0    // last element must always be 0
            };

    } // namespace Handler

} // namespace Osmium
