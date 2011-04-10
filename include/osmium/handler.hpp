#ifndef OSMIUM_HANDLER_HPP
#define OSMIUM_HANDLER_HPP

namespace Osmium {

    /**
     * @brief Namespace for callback handlers.
     */
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

            void callback_init() const {
            }

            void callback_object(OSM::Object *) const {
            }

            void callback_before_nodes() const {
            }

            void callback_node(OSM::Node *) const {
            }

            void callback_after_nodes() const {
            }

            void callback_before_ways() const {
            }

            void callback_way(OSM::Way *) const {
            }

            void callback_after_ways() const {
            }

            void callback_before_relations() const {
            }

            void callback_relation(OSM::Relation *) const {
            }

            void callback_after_relations() const {
            }

            void callback_multipolygon(OSM::Multipolygon *) const {
            }

            void callback_final() const {
            }

        }; // class Base

    } // namespace Handler

} // namespace Osmium

#endif // OSMIUM_HANDLER_HPP
