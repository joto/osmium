/*

  Shows how the Osmium::Relations::Assembler class is used. Collects
  all members of all relations in the input data and dumps the tags of
  all relations and all their members to stdout.

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

class DebugRelationsAssembler : public Osmium::Relations::Assembler<DebugRelationsAssembler, Osmium::Relations::RelationInfo> {

    typedef Osmium::Relations::Assembler<DebugRelationsAssembler, Osmium::Relations::RelationInfo> AssemblerType;

    typedef AssemblerType::HandlerPass1                   HandlerPass1;
    typedef AssemblerType::HandlerPass2<true, true, true> HandlerPass2;

    HandlerPass1 m_handler_pass1;
    HandlerPass2 m_handler_pass2;

public:

    DebugRelationsAssembler() :
        Osmium::Relations::Assembler<DebugRelationsAssembler, Osmium::Relations::RelationInfo>(),
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
        add_relation(Osmium::Relations::RelationInfo(relation));
    }

    bool keep_member(Osmium::Relations::RelationInfo& /*relation_info*/, const Osmium::OSM::RelationMember& /*member*/) {
        return true;
    }

    void complete_relation(Osmium::Relations::RelationInfo& relation_info) {
        static const char* types[] = { "node", "way", "relation" };

        std::cout << "Relation " << relation_info.relation()->id() << "\n";
        BOOST_FOREACH(const Osmium::OSM::Tag& tag, relation_info.relation()->tags()) {
            std::cout << "  " << tag.key() << "=" << tag.value() << "\n";
        }

        int i = 0;
        const Osmium::OSM::RelationMemberList& rml = relation_info.relation()->members();
        BOOST_FOREACH(const shared_ptr<Osmium::OSM::Object const>& object, relation_info.members()) {
            std::cout << "  Member " << types[object->get_type()] << " " << object->id() << " with role '" << rml[i].role() << "'\n";
            BOOST_FOREACH(const Osmium::OSM::Tag& tag, object->tags()) {
                std::cout << "    " << tag.key() << "=" << tag.value() << "\n";
            }
            ++i;
        }
        std::cout << "\n";
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

