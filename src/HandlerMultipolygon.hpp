#ifndef OSMIUM_HANDLER_MULTIPOLYGON_HPP
#define OSMIUM_HANDLER_MULTIPOLYGON_HPP

namespace Osmium {

    namespace Handler {

        class Multipolygon : public Base {

          public:

            // in pass 1
            void callback_before_relations() {
            }

            // in pass 1
            void callback_relation(const OSM::Relation *object) {
            }

            // in pass 1
            void callback_after_relations() {
            }

            // in pass 2
            void callback_way(const OSM::Way *object) {
            }

            // in pass 2
            void callback_after_ways() {
            }

        }; // class Multipolygon

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_MULTIPOLYGON_HPP
