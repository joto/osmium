#ifndef OSMIUM_HANDLER_HPP
#define OSMIUM_HANDLER_HPP

namespace Osmium {

    namespace Handler {

        // Base class for all handler classes.
        // Defines empty methods that can be overwritten in child classes.
        class Base {

            bool debug;

        public:

            Base(bool debug) : debug(debug) {
            }

            void callback_init() {
            }

            void callback_object(OSM::Object *) {
            }

            void callback_before_nodes() {
            }

            void callback_node(OSM::Node *) {
            }

            void callback_after_nodes() {
            }

            void callback_before_ways() {
            }

            void callback_way(OSM::Way *) {
            }

            void callback_after_ways() {
            }

            void callback_before_relations() {
            }

            void callback_relation(OSM::Relation *) {
            }

            void callback_after_relations() {
            }

            void callback_final() {
            }

        }; // class Base

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_HPP
