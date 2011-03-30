#ifndef OSMIUM_INPUT_HPP
#define OSMIUM_INPUT_HPP

namespace Osmium {

    namespace Input {

        class Base {

          public:
            
            OSM::Node     *node;
            OSM::Way      *way;
            OSM::Relation *relation;

            Base() {
                node     = new Osmium::OSM::Node;
                way      = new Osmium::OSM::Way;
                relation = new Osmium::OSM::Relation;
            }

            virtual ~Base() {
                delete relation;
                delete way;
                delete node;
            }

            virtual void parse() = 0;

        };

    } // namespace Input

} // namespace Osmium

#endif // OSMIUM_INPUT_HPP
