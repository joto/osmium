#ifndef OSMIUM_INPUT_HPP
#define OSMIUM_INPUT_HPP

#include <Handler.hpp>

namespace Osmium {

    namespace Input {

        template <class THandler>
        class Base {

          public:
            
            OSM::Node     *node;
            OSM::Way      *way;
            OSM::Relation *relation;

            THandler *handler;

            Base(THandler *h) : handler(h) {
                node     = new Osmium::OSM::Node;
                way      = new Osmium::OSM::Way;
                relation = new Osmium::OSM::Relation;

                if (! handler) {
                    handler = new THandler;
                }

                handler->callback_init();
            }

            virtual ~Base() {
                handler->callback_final();

                delete handler;

                delete relation;
                delete way;
                delete node;
            }

            virtual void parse() = 0;

        };

    } // namespace Input

} // namespace Osmium

#include <XMLParser.hpp>
#include <PBFParser.hpp>

#endif // OSMIUM_INPUT_HPP
