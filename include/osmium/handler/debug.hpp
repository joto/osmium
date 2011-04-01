#ifndef OSMIUM_HANDLER_DEBUG_HPP
#define OSMIUM_HANDLER_DEBUG_HPP

namespace Osmium {

    namespace Handler {

        class Debug : public Base {

          public:

            Debug() : Base() {
            }

            void callback_object(OSM::Object *object) const {
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

            void callback_node(const OSM::Node *object) const {
                std::cout << "  node:" << std::endl;
                std::cout << "    lon=" << object->get_lon() << std::endl;
                std::cout << "    lat=" << object->get_lat() << std::endl;
            }

            void callback_way(const OSM::Way *object) const {
                std::cout << "  way:" << std::endl;
                std::cout << "    node_count=" << object->node_count() << std::endl;
                std::cout << "    nodes:" << std::endl;
                for (int i=0; i < object->node_count(); i++) {
                    std::cout << "      ref=" << object->nodes[i] << std::endl;
                }
            }

            void callback_relation(OSM::Relation *object) const {
                std::cout << "  relation:" << std::endl;
                std::cout << "    member_count=" << object->member_count() << std::endl;
                std::cout << "    members:" << std::endl;
                for (int i=0; i < object->member_count(); i++) {
                    const Osmium::OSM::RelationMember *m = object->get_member(i);
                    std::cout << "      type=" << m->type << " ref=" << m->ref << " role=|" << m->role << "|" << std::endl;
                }
            }

        }; // class Debug

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_DEBUG_HPP
