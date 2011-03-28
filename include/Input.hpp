#ifndef OSMIUM_INPUT_HPP
#define OSMIUM_INPUT_HPP

namespace Osmium {

    namespace Input {

        class Base {

          public:
            
            virtual void parse(Osmium::OSM::Node *in_node, Osmium::OSM::Way *in_way, Osmium::OSM::Relation *in_relation) = 0;

        };

    } // namespace Input

} // namespace Osmium

#endif // OSMIUM_INPUT_HPP
