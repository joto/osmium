#ifndef OSMIUM_HANDLER_DEBUG_HPP
#define OSMIUM_HANDLER_DEBUG_HPP

namespace Osmium {

    namespace Handler {

        class Debug : public Base {

          public:

            Debug() : Base() {
            }

            void callback_object(OSM::Object *object) {
                std::cout << "object:" << std::endl;
                std::cout << "  id="   << object->get_id() << std::endl;
                std::cout << "  version="   << object->get_version() << std::endl;
                std::cout << "  uid="       << object->get_uid() << std::endl;
                std::cout << "  user=|"      << object->get_user() << "|" << std::endl;
                std::cout << "  changeset=" << object->get_changeset() << std::endl;
                std::cout << "  timestamp=" << object->get_timestamp_str() << std::endl;
                std::cout << "  tags:" << std::endl;
                for (int i=0; i < object->tag_count(); i++) {
                    std::cout << "    k=|" << object->get_tag_key(i) << "| v=|" << object->get_tag_value(i) << "|" << std::endl;
                }
            }

            void callback_node(const OSM::Node *object) {
                std::cout << "  node:" << std::endl;
                std::cout << "    lon=" << object->get_lon() << std::endl;
                std::cout << "    lat=" << object->get_lat() << std::endl;
            }

            void callback_way(const OSM::Way *object) {
                std::cout << "  way:" << std::endl;
                std::cout << "    node_count=" << object->node_count() << std::endl;
                std::cout << "    XXX output node list" << std::endl;
            }

            void callback_relation(const OSM::Relation *object) {
                std::cout << "  relation:" << std::endl;
                std::cout << "    XXX output member info" << std::endl;
            }

            void callback_final() {
            }

        }; // class Debug

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_DEBUG_HPP
