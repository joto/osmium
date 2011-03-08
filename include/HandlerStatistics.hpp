#ifndef OSMIUM_HANDLER_STATISTICS_HPP
#define OSMIUM_HANDLER_STATISTICS_HPP

#include "Sqlite.hpp"

namespace Osmium {

    namespace Handler {

        class Statistics : public Base {

            // if you change anything in this struct, also change the corresponding array in Statistics.cpp
            struct statistics {
                uint64_t nodes;
                uint64_t nodes_without_tags;
                uint64_t node_tags;
                uint64_t max_node_id;
                uint64_t max_tags_on_node;
                uint64_t ways;
                uint64_t way_tags;
                uint64_t way_nodes;
                uint64_t max_way_id;
                uint64_t max_tags_on_way;
                uint64_t max_nodes_on_way;
                uint64_t closed_ways;
                uint64_t relations;
                uint64_t relation_tags;
                uint64_t relation_members;
                uint64_t max_relation_id;
                uint64_t max_tags_on_relation;
                uint64_t max_members_on_relation;
                uint64_t users;
                uint64_t max_user_id;
                uint64_t anon_user_objects;
                uint64_t max_node_version;
                uint64_t max_way_version;
                uint64_t max_relation_version;
                uint64_t sum_node_version;
                uint64_t sum_way_version;
                uint64_t sum_relation_version;
                uint64_t max_changeset_id;
            } stats;

            static const char *stat_names[];

            osm_object_id_t id;
            osm_version_t   version;
            int             tag_count;

            Sqlite::Database *db;

        public:

            Statistics() : Base() {
                // initialize all statistics to zero
                for (int i=0; stat_names[i]; i++) {
                    ((uint64_t *) &stats)[i] = 0;
                }
            }

            void callback_object(const OSM::Object *object) {
                id        = object->get_id();
                version   = object->get_version();
                tag_count = object->tag_count();

                osm_user_id_t uid = object->get_uid();
                if (uid == 0)
                    stats.anon_user_objects++;
                if (uid > (int64_t) stats.max_user_id)
                    stats.max_user_id = uid;

                osm_changeset_id_t changeset = object->get_changeset();
                if (changeset > (int64_t) stats.max_changeset_id)
                    stats.max_changeset_id = changeset;
            }

            void callback_node(const OSM::Node * /*object*/) {
                stats.nodes++;
                if (tag_count == 0)
                    stats.nodes_without_tags++;
                if (id > (int64_t) stats.max_node_id)
                    stats.max_node_id = id;
                stats.node_tags += tag_count;
                if (tag_count > (int64_t) stats.max_tags_on_node)
                    stats.max_tags_on_node = tag_count;
                if (version > (int64_t) stats.max_node_version)
                    stats.max_node_version = version;
                stats.sum_node_version += version;
            }

            void callback_way(const OSM::Way *object) {
                stats.ways++;
                if (object->is_closed())
                    stats.closed_ways++;
                if (id > (int64_t) stats.max_way_id)
                    stats.max_way_id = id;
                stats.way_tags += tag_count;
                stats.way_nodes += object->node_count();
                if (tag_count > (int64_t) stats.max_tags_on_way)
                    stats.max_tags_on_way = tag_count;
                if (object->node_count() > (int64_t) stats.max_nodes_on_way)
                    stats.max_nodes_on_way = object->node_count();
                if (version > (int64_t) stats.max_way_version)
                    stats.max_way_version = version;
                stats.sum_way_version += version;
            }

            void callback_relation(const OSM::Relation *object) {
                stats.relations++;
                if (id > (int64_t) stats.max_relation_id)
                    stats.max_relation_id = id;
                stats.relation_tags += tag_count;
                osm_sequence_id_t member_count = object->member_count();
                stats.relation_members += member_count;
                if (tag_count > (int64_t) stats.max_tags_on_relation)
                    stats.max_tags_on_relation = tag_count;
                if (member_count > (int64_t) stats.max_members_on_relation)
                    stats.max_members_on_relation = member_count;
                if (version > (int64_t) stats.max_relation_version)
                    stats.max_relation_version = version;
                stats.sum_relation_version += version;
            }

            void callback_final() {
                unlink("count.db");
                db = new Sqlite::Database("count.db");

                sqlite3 *sqlite_db = db->get_sqlite3();
                if (SQLITE_OK != sqlite3_exec(sqlite_db, \
                    "CREATE TABLE stats (" \
                    "  key    TEXT, " \
                    "  value  INT64 " \
                    ");", 0, 0, 0)) {
                    std::cerr << "Database error: " << sqlite3_errmsg(sqlite_db) << "\n";
                    sqlite3_close(sqlite_db);
                    exit(1);
                }

                Sqlite::Statement *statement_insert_into_main_stats = db->prepare("INSERT INTO stats (key, value) VALUES (?, ?);");
                db->begin_transaction();

//                std::ofstream out_stats("stats.csv");
                for (int i=0; stat_names[i]; i++) {
//                    out_stats << stat_names[i] << '\t' << ((uint64_t *) &stats)[i] << '\n';
                    statement_insert_into_main_stats
                        ->bind_text(stat_names[i])
                        ->bind_int64( ((uint64_t *) &stats)[i] )
                        ->execute();
                }
                statement_insert_into_main_stats
                        ->bind_text("nodes_with_tags")
                        ->bind_int64( ((uint64_t *) &stats)[0] - ((uint64_t *) &stats)[1] )
                        ->execute();
                        
//                out_stats.close();

                db->commit();

                db->close();
            }

        }; // class Statistics

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_STATISTICS_HPP
