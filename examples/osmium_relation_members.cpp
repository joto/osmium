/*

  Example tool that dumps information about relations and their members.

*/

/*

Copyright 2012 Jochen Topf <jochen@topf.org> and others (see README).

This file is part of Osmium (https://github.com/joto/osmium).

Osmium is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License or (at your option) the GNU
General Public License as published by the Free Software Foundation, either
version 3 of the Licenses, or (at your option) any later version.

Osmium is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License and the GNU
General Public License for more details.

You should have received a copy of the Licenses along with Osmium. If not, see
<http://www.gnu.org/licenses/>.

*/

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/relations/assembler.hpp>

class DebugRelationsAssembler : public Osmium::Relations::Assembler<Osmium::Relations::RelationInfo, DebugRelationsAssembler> {

    typedef Osmium::Relations::Assembler<Osmium::Relations::RelationInfo, DebugRelationsAssembler> AssemblerType;

    typedef AssemblerType::HandlerPass1                     HandlerPass1;
    typedef AssemblerType::HandlerPass2<true, false, false> HandlerPass2;

    HandlerPass1 m_handler_pass1;
    HandlerPass2 m_handler_pass2;

public:

    DebugRelationsAssembler() :
        Osmium::Relations::Assembler<Osmium::Relations::RelationInfo, DebugRelationsAssembler>(),
        m_handler_pass1(*this),
        m_handler_pass2(*this) {
    }

    HandlerPass1& handler_pass1() {
        return m_handler_pass1;
    }

    HandlerPass2& handler_pass2() {
        return m_handler_pass2;
    }

    void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
        if (relation->id() == 2952) {
            return;
        }
        std::cerr << "Relation found: " << relation->id() << " with " << relation->members().size() << " members\n";
        add_relation(Osmium::Relations::RelationInfo(relation));
    }

    bool keep_member(Osmium::Relations::RelationInfo& /*relation_info*/, const Osmium::OSM::RelationMember& member) {
        if (member.type() == 'n') {
            return true;
        }
        return false;
    }

    void node_not_in_any_relation(const shared_ptr<Osmium::OSM::Node const>& node) {
//        std::cerr << "Node " << node->id() << " not in any relation\n";
    }

    void complete_relation(Osmium::Relations::RelationInfo& relation_info) {
        std::cerr << "Relation completed: " << relation_info.relation()->id() << "\n";

        BOOST_FOREACH(shared_ptr<Osmium::OSM::Object const>& object, relation_info.members()) {
            std::cerr << "  Member: " << object->id() << "\n";
        }
    }

};

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    DebugRelationsAssembler assembler;

    Osmium::OSMFile infile(argv[1]);
    Osmium::Input::read(infile, assembler.handler_pass1());
    Osmium::Input::read(infile, assembler.handler_pass2());
}

