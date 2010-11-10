#ifndef OSMIUM_HANDLER_TAGSTATS_HPP
#define OSMIUM_HANDLER_TAGSTATS_HPP

#include <google/sparse_hash_map>
#include <map>
#include <bitset>
#include <string>

#include <gd.h>

#include "Sqlite.hpp"

struct eqstr {
    bool operator()(const char* s1, const char* s2) const {
        return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
    }
};

struct less_str {
    bool operator()(const char* s1, const char* s2) const {
        return strcmp(s1, s2) < 0;
    }
};

union counter_t {
    uint32_t count[4];
    struct {
        uint32_t all;
        uint32_t nodes;
        uint32_t ways;
        uint32_t relations;
    } by_type;
};

typedef google::sparse_hash_map<const char *, counter_t, HASH_NAMESPACE::hash<const char*>, eqstr> value_hash_map;
typedef google::sparse_hash_map<osm_user_id_t, uint32_t> user_hash_map;

// XXX these do not work (segfault) for some reason
//typedef std::map<const char *, counter_t, less_str> value_hash_map;
//typedef std::map<osm_user_id_t, uint32_t> user_hash_map;

namespace Osmium {

    class ObjectTagStat {

    public:

        counter_t key;
        counter_t values;
        counter_t users;

        value_hash_map values_stat;
        value_hash_map keypairs_stat;
        user_hash_map  users_stat;

        static const int location_image_y_size = 360;
        static const int location_image_x_size = 2 * location_image_y_size;

        std::bitset<location_image_x_size * location_image_y_size> location;

        int grids;

        ObjectTagStat() {
            memset(&key,    0, sizeof(key));
            memset(&values, 0, sizeof(key));
            memset(&users,  0, sizeof(key));
            grids = 0;
        }

    }; // class ObjectTagStat

    namespace Handler {

        typedef google::sparse_hash_map<const char *, ObjectTagStat *, HASH_NAMESPACE::hash<const char*>, eqstr> tag_hash_map;
        //typedef std::map<const char *, ObjectTagStat *, less_str> tag_hash_map;

        class TagStats : public Base {

            time_t timer;

            tag_hash_map             tags_stat;
            tag_hash_map::iterator   tags_iterator;
            value_hash_map::iterator values_iterator;
            user_hash_map::iterator  users_iterator;

            char max_timestamp[Osmium::OSM::Object::max_length_timestamp];

            // this must be much bigger than the largest string we want to store
            static const int string_store_size = 1024 * 1204 * 10;
            StringStore *string_store;

            Sqlite::Database *db;

        public:

            TagStats() {
                string_store = new StringStore(string_store_size);
                max_timestamp[0] = 0;
            }

            void update_tag_stats(ObjectTagStat *stat, const char * /* key */, const char *value, OSM::Object *object) {
                stat->key.count[0]++;
                stat->key.count[object->type()]++;

                values_iterator = stat->values_stat.find(value);
                if (values_iterator == stat->values_stat.end()) {
                    const char *ptr = string_store->add(value);
                    counter_t counter;
                    counter.count[0]              = 1;
                    counter.count[1]              = 0;
                    counter.count[2]              = 0;
                    counter.count[3]              = 0;
                    counter.count[object->type()] = 1;
                    stat->values_stat.insert(std::pair<const char *, counter_t>(ptr, counter));
                    stat->values.count[0]++;
                    stat->values.count[object->type()]++;
                } else {
                    values_iterator->second.count[0]++;
                    values_iterator->second.count[object->type()]++;
                    if (values_iterator->second.count[object->type()] == 1) {
                        stat->values.count[object->type()]++;
                    }
                }

                stat->users_stat[object->get_uid()]++;

                if (object->type() == NODE) {
                    int x =                                                int(2 * (((OSM::Node *)object)->get_lon() + 180));
                    int y = Osmium::ObjectTagStat::location_image_y_size - int(2 * (((OSM::Node *)object)->get_lat() +  90));
                    stat->location[Osmium::ObjectTagStat::location_image_x_size * y + x] = true;
                }
            }

            void callback_object(OSM::Object *object) {
                ObjectTagStat *stat;

                if (strcmp(max_timestamp, object->timestamp_str) < 0) {
                    memccpy(max_timestamp, object->timestamp_str, 0, Osmium::OSM::Object::max_length_timestamp);
                }

                int tag_count = object->tag_count();
                for (int i=0; i<tag_count; i++) {
                    const char* key = object->get_tag_key(i);

                    tags_iterator = tags_stat.find(key);
                    if (tags_iterator == tags_stat.end()) {
                        stat = new ObjectTagStat();
                        tags_stat.insert(std::pair<const char *, ObjectTagStat *>(string_store->add(key), stat));
                    } else {
                        stat = tags_iterator->second;
                    }
                    update_tag_stats(stat, key, object->get_tag_value(i), object);
                }

                // count key pairs
                for (int i=0; i<tag_count; i++) {
                    for (int j=i+1; j<tag_count; j++) {
                        const char *first  = object->get_tag_key(i);
                        const char *second = object->get_tag_key(j);
                        const char *min, *max;
                        if (strcmp(first, second) < 0) { // a strcmp is not strictly necessary here, optimize?
                            min = first;
                            max = second;
                        } else {
                            min = second;
                            max = first;
                        }
                        stat = tags_stat.find(min)->second;
                        const char *key = tags_stat.find(max)->first;

                        values_iterator = stat->keypairs_stat.find(key);
                        if (values_iterator == stat->keypairs_stat.end()) {
                            counter_t counter;
                            counter.count[0]              = 1;
                            counter.count[1]              = 0;
                            counter.count[2]              = 0;
                            counter.count[3]              = 0;
                            counter.count[object->type()] = 1;
                            stat->keypairs_stat.insert(std::pair<const char *, counter_t>(key, counter));
                        } else {
                            values_iterator->second.count[0]++;
                            values_iterator->second.count[object->type()]++;
                        }

                    }
                }
            }

            void print_images() {
                std::bitset<Osmium::ObjectTagStat::location_image_x_size * Osmium::ObjectTagStat::location_image_y_size> location_all;
                int sum_size=0;

                Sqlite::Statement *statement_insert_into_key_distributions = db->prepare("INSERT INTO key_distributions (key, png) VALUES (?, ?);");
                db->begin_transaction();

                for (tags_iterator = tags_stat.begin(); tags_iterator != tags_stat.end(); tags_iterator++) {
                    const char *key = tags_iterator->first;
                    ObjectTagStat *stat = tags_iterator->second;

                    gdImagePtr im = gdImageCreate(Osmium::ObjectTagStat::location_image_x_size, Osmium::ObjectTagStat::location_image_y_size);
                    int bgColor = gdImageColorAllocate(im, 0, 0, 0);
                    gdImageColorTransparent(im, bgColor);
                    int fgColor = gdImageColorAllocate(im, 180, 0, 0);

                    int n=0;
                    for (int y=0; y < Osmium::ObjectTagStat::location_image_y_size; y++) {
                        for (int x=0; x < Osmium::ObjectTagStat::location_image_x_size; x++) {
                            if (stat->location[n]) {
                                stat->grids++;
                                gdImageSetPixel(im, x, y, fgColor);
                                location_all[n] = true;
                            }
                            n++;
                        }
                    }

                    int size;
                    void *ptr = gdImagePngPtr(im, &size);
                    sum_size += size;
                    statement_insert_into_key_distributions
                        ->bind_text(key)
                        ->bind_blob(ptr, size)
                        ->execute();
                    gdFree(ptr);

                    gdImageDestroy(im);
                }

                gdImagePtr im_all = gdImageCreate(Osmium::ObjectTagStat::location_image_x_size, Osmium::ObjectTagStat::location_image_y_size);
                gdImageColorAllocate(im_all, 0, 0, 0);
                int white_all = gdImageColorAllocate(im_all, 255, 255, 255);
                int n=0, count_grid=0;
                for (int y=0; y < Osmium::ObjectTagStat::location_image_y_size; y++) {
                    for (int x=0; x < Osmium::ObjectTagStat::location_image_x_size; x++) {
                        if (location_all[n]) {
                            gdImageSetPixel(im_all, x, y, white_all);
                            count_grid++;
                        }
                        n++;
                    }
                }
                std::cerr << "grids_all: " << count_grid << "\n";

                int size;
                void *ptr = gdImagePngPtr(im_all, &size);
                statement_insert_into_key_distributions
                    ->bind_null()
                    ->bind_blob(ptr, size)
                    ->execute();
                gdFree(ptr);

                gdImageDestroy(im_all);

                std::cerr << "sum of location image sizes: " << sum_size + size << "\n";

                db->commit();
            }

            void print_memory_usage() {
                std::cerr << "string_store: chunk_size=" << string_store->get_chunk_size() / 1024 << "kB"
                          <<                  " chunks=" << string_store->get_chunk_count()
                          <<                  " memory=" << string_store->get_chunk_size() * string_store->get_chunk_count() / 1024 << "kB"
                          <<           " bytes_in_last=" << string_store->get_used_bytes_in_last_chunk() / 1024 << "kB"
                          << "\n";

                std::cerr << "sizeof(ObjectTagStat)=" << sizeof(ObjectTagStat) << "\n";

                char filename[100];
                sprintf(filename, "/proc/%d/status", getpid());
                std::ifstream status_file(filename);
                std::string line;

                if (status_file.is_open()) {
                    while (! status_file.eof() ) {
                        std::getline(status_file, line);
                        if (line.substr(0, 6) == "VmPeak" || line.substr(0, 6) == "VmSize") {
                            std::cerr << line << '\n';
                        }
                    }
                    status_file.close();
                }

            }

            void timer_info(const char *msg) {
                int duration = time(0) - timer;
                std::cout << msg << " took " << duration << " seconds (about " << duration / 60 << " minutes)\n";
            }

            void callback_before_nodes() {
                timer = time(0);
            }

            void callback_after_nodes() {
                timer_info("processing nodes");
                print_memory_usage();
                timer = time(0);
                print_images();
                timer_info("dumping images");
                print_memory_usage();
            }

            void callback_before_ways() {
                timer = time(0);
            }

            void callback_after_ways() {
                timer_info("processing ways");
            }

            void callback_before_relations() {
                timer = time(0);
            }

            void callback_after_relations() {
                timer_info("processing relations");
            }

            void callback_init() {
                db = new Sqlite::Database("taginfo-db.db");

                std::cerr << "sizeof(counter_t) = " << sizeof(counter_t) << "\n";
                std::cerr << "sizeof(value_hash_map) = " << sizeof(value_hash_map) << "\n";
                std::cerr << "sizeof(user_hash_map) = " << sizeof(user_hash_map) << "\n";
                std::cerr << "sizeof(std::bitset<x_size*y_size>) = " << sizeof(std::bitset<Osmium::ObjectTagStat::location_image_x_size * Osmium::ObjectTagStat::location_image_y_size>) << "\n";
                std::cerr << "sizeof(ObjectTagStat) = " << sizeof(ObjectTagStat) << "\n\n";

                print_memory_usage();
                std::cerr << "init done\n\n";
            }

            void callback_final() {
                print_memory_usage();
                timer = time(0);

                Sqlite::Statement *statement_insert_into_keys = db->prepare("INSERT INTO keys (key, " \
                    " count_all,  count_nodes,  count_ways,  count_relations, " \
                    "values_all, values_nodes, values_ways, values_relations, " \
                    " users_all,  users_nodes,  users_ways,  users_relations, " \
                    "grids) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");

                Sqlite::Statement *statement_insert_into_tags = db->prepare("INSERT INTO tags (key, value, " \
                    "count_all, count_nodes, count_ways, count_relations) " \
                    "VALUES (?, ?, ?, ?, ?, ?);");

                Sqlite::Statement *statement_insert_into_keypairs = db->prepare("INSERT INTO keypairs (key1, key2, " \
                    "count_all, count_nodes, count_ways, count_relations) " \
                    "VALUES (?, ?, ?, ?, ?, ?);");

                Sqlite::Statement *statement_update_meta = db->prepare("UPDATE meta SET data_until=(date(?) || ' ' || time(?))");

                db->begin_transaction();

                statement_update_meta->bind_text(max_timestamp)->bind_text(max_timestamp)->execute();

                long tags_hash_map_size=tags_stat.size();
                long tags_hash_map_buckets=tags_stat.bucket_count();

                long values_hash_map_size=0;
                long values_hash_map_buckets=0;

                long keypairs_hash_map_size=0;
                long keypairs_hash_map_buckets=0;

                long users_hash_map_size=0;
                long users_hash_map_buckets=0;

                for (tags_iterator = tags_stat.begin(); tags_iterator != tags_stat.end(); tags_iterator++) {
                    ObjectTagStat *stat = tags_iterator->second;
    
                    values_hash_map_size    += stat->values_stat.size();
                    values_hash_map_buckets += stat->values_stat.bucket_count();

                    for (values_iterator = stat->values_stat.begin(); values_iterator != stat->values_stat.end(); values_iterator++) {
                        statement_insert_into_tags
                            ->bind_text(tags_iterator->first)
                            ->bind_text(values_iterator->first)
                            ->bind_int(values_iterator->second.by_type.all)
                            ->bind_int(values_iterator->second.by_type.nodes)
                            ->bind_int(values_iterator->second.by_type.ways)
                            ->bind_int(values_iterator->second.by_type.relations)
                            ->execute();
                    }

                    stat->users.by_type.all = stat->users_stat.size();
                    users_hash_map_size    += stat->users.by_type.all;
                    users_hash_map_buckets += stat->users_stat.bucket_count();

                    statement_insert_into_keys
                        ->bind_text(tags_iterator->first)
                        ->bind_int(stat->key.by_type.all)
                        ->bind_int(stat->key.by_type.nodes)
                        ->bind_int(stat->key.by_type.ways)
                        ->bind_int(stat->key.by_type.relations)
                        ->bind_int(stat->values.by_type.all)
                        ->bind_int(stat->values.by_type.nodes)
                        ->bind_int(stat->values.by_type.ways)
                        ->bind_int(stat->values.by_type.relations)
                        ->bind_int(stat->users.by_type.all)
                        ->bind_int(stat->users.by_type.nodes)
                        ->bind_int(stat->users.by_type.ways)
                        ->bind_int(stat->users.by_type.relations)
                        ->bind_int(stat->grids)
                        ->execute();

                    keypairs_hash_map_size    += stat->keypairs_stat.size();
                    keypairs_hash_map_buckets += stat->keypairs_stat.bucket_count();

                    for (values_iterator = stat->keypairs_stat.begin(); values_iterator != stat->keypairs_stat.end(); values_iterator++) {
                        statement_insert_into_keypairs
                            ->bind_text(tags_iterator->first)
                            ->bind_text(values_iterator->first)
                            ->bind_int(values_iterator->second.by_type.all)
                            ->bind_int(values_iterator->second.by_type.nodes)
                            ->bind_int(values_iterator->second.by_type.ways)
                            ->bind_int(values_iterator->second.by_type.relations)
                            ->execute();
                    }

                }

                db->commit();
                db->close();
                timer_info("dumping to db");

                std::cerr << "\nhash map sizes:\n";
                std::cerr << "  tags:     size=" <<     tags_hash_map_size << " buckets=" <<     tags_hash_map_buckets << " sizeof(ObjectTagStat)=" << sizeof(ObjectTagStat) << " *=" <<     tags_hash_map_size * sizeof(ObjectTagStat) << "\n";
                std::cerr << "  values:   size=" <<   values_hash_map_size << " buckets=" <<   values_hash_map_buckets << " sizeof(counter_t)="     << sizeof(counter_t)     << " *=" <<   values_hash_map_size * sizeof(counter_t) << "\n";
                std::cerr << "  keypairs: size=" << keypairs_hash_map_size << " buckets=" << keypairs_hash_map_buckets << " sizeof(counter_t)="     << sizeof(counter_t)     << " *=" << keypairs_hash_map_size * sizeof(counter_t) << "\n";
                std::cerr << "  users:    size=" <<    users_hash_map_size << " buckets=" <<    users_hash_map_buckets << " sizeof(uint32_t)="      << sizeof(uint32_t)      << " *=" <<    users_hash_map_size * sizeof(uint32_t) << "\n";
                std::cerr << "  sum: " << tags_hash_map_size * sizeof(ObjectTagStat) + values_hash_map_size * sizeof(counter_t) + keypairs_hash_map_size * sizeof(counter_t) + users_hash_map_size * sizeof(uint32_t) << "\n";

                std::cerr << "\ntotal memory for hashes:\n";
                std::cerr << "  (sizeof(hash key) + sizeof(hash value *) + 2.5 bit overhead) * bucket_count + sizeof(hash value) * size \n";
                std::cerr << " tags:     " << ((sizeof(const char*)*8 + sizeof(ObjectTagStat *)*8 + 3) * tags_hash_map_buckets / 8 ) + sizeof(ObjectTagStat) * tags_hash_map_size << "\n";
                std::cerr << "  (sizeof(hash key) + sizeof(hash value  ) + 2.5 bit overhead) * bucket_count\n";
                std::cerr << " values:   " << ((sizeof(const char*)*8 + sizeof(counter_t)*8 + 3) * values_hash_map_buckets / 8 ) << "\n";
                std::cerr << " keypairs: " << ((sizeof(const char*)*8 + sizeof(counter_t)*8 + 3) * keypairs_hash_map_buckets / 8 ) << "\n";
                std::cerr << " users:    " << ((sizeof(osm_user_id_t)*8 + sizeof(uint32_t)*8 + 3) * users_hash_map_buckets / 8 )  << "\n";
                std::cerr << "\n";

                print_memory_usage();
            }

        }; // class TagStats

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_TAGSTATS_HPP
