#ifndef OSMIUM_INPUT_HPP
#define OSMIUM_INPUT_HPP

#include <osmium/handler.hpp>

namespace Osmium {

    namespace Input {

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
