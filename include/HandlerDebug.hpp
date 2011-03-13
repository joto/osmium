#ifndef OSMIUM_HANDLER_DEBUG_HPP
#define OSMIUM_HANDLER_DEBUG_HPP

namespace Osmium {

    namespace Handler {

        class Debug : public Base {

          public:

            Debug() : Base() {
            }

            void callback_object(OSM::Object *object) {
                std::cout << "object:\n";
                std::cout << "  id="   << object->get_id() << "\n";
                std::cout << "  version="   << object->get_version() << "\n";
                std::cout << "  uid="       << object->get_uid() << "\n";
                std::cout << "  user=|"      << object->get_user() << "|\n";
                std::cout << "  changeset=" << object->get_changeset() << "\n";
                std::cout << "  timestamp=" << object->get_timestamp_str() << "\n";
                std::cout << "  tags:\n";
                for (int i=0; i < object->tag_count(); i++) {
                    std::cout << "    k=|" << object->get_tag_key(i) << "| v=|" << object->get_tag_value(i) << "|\n";
                }
            }

            void callback_node(const OSM::Node *object) {
                std::cout << "  node:\n";
                std::cout << "    lon=" << object->get_lon() << "\n";
                std::cout << "    lat=" << object->get_lat() << "\n";
            }

            void callback_way(const OSM::Way *object) {
                std::cout << "  way:\n";
                std::cout << "    node_count=" << object->node_count() << "\n";
                std::cout << "    XXX output node list\n";
            }

            void callback_relation(const OSM::Relation *object) {
                std::cout << "  relation:\n";
                std::cout << "    XXX output member info\n";
            }

            void callback_final() {
            }

        }; // class Debug

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_DEBUG_HPP
