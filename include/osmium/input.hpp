#ifndef OSMIUM_INPUT_HPP
#define OSMIUM_INPUT_HPP

#include <osmium/handler.hpp>

namespace Osmium {

    /**
     * @brief Namespace for input classes implementing file parsers.
     */
    namespace Input {

        /**
         * Handlers can throw this exception to show that they are done.
         * When a handler, for instance, is only interested in nodes, it
         * can throw this in the after_nodes() callback. The parser will
         * stop reading the input file after this.
         *
         * Note that when you write a handler that calls other handlers
         * that can throw this, you might have to catch this exception
         * in your handler.
         */
        class StopReading {
        };

        template <class THandler>
        class Base {

          public:
            
            OSM::Node     *node;
            OSM::Way      *way;
            OSM::Relation *relation;

            THandler *handler;
            bool delete_handler_on_destruction;

            Base(THandler *h) : handler(h) {
                node     = new Osmium::OSM::Node;
                way      = new Osmium::OSM::Way;
                relation = new Osmium::OSM::Relation;

                if (handler) {
                    delete_handler_on_destruction = false;
                } else {
                    handler = new THandler;
                    delete_handler_on_destruction = true;
                }

                handler->callback_init();
            }

            virtual ~Base() {
                handler->callback_final();

                if (delete_handler_on_destruction) {
                    delete handler;
                }

                delete relation;
                delete way;
                delete node;
            }

            virtual void parse() = 0;

        };

    } // namespace Input

} // namespace Osmium

#include <osmium/input/xml.hpp>
#include <osmium/input/pbf.hpp>

#endif // OSMIUM_INPUT_HPP
