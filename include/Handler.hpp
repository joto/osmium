#ifndef OSMIUM_HANDLER_HPP
#define OSMIUM_HANDLER_HPP

namespace Osmium {

    namespace Handler {

        /**
         * Base class for all handler classes.
         * Defines empty methods that can be overwritten in child classes.
         *
         * To define your own handler create a subclass of this class.
         * Only overwrite the functions you actually use. They must be declared public.
         * If you overwrite the constructor call the Base constructor without arguments
         * and make sure you have at least a constructor that takes no arguments.
         */
        class Base {

          public:

            Base() {
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

            void callback_multipolygon(OSM::Multipolygon *) {
            }

            void callback_final() {
            }

        }; // class Base

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_HPP
