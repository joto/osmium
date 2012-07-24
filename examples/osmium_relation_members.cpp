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

#include <iostream>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>
#include <osmium/relations/assembler.hpp>

class DebugRelationsAssembler : public Osmium::Relations::Assembler<DebugRelationsAssembler, Osmium::Relations::RelationInfo, true, true, true> {

    typedef Osmium::Relations::Assembler<DebugRelationsAssembler, Osmium::Relations::RelationInfo, true, true, true> AssemblerType;

    /**
     * Dump information about a relation with all its members to stdout.
     */
    void dump_relation(const Osmium::Relations::RelationInfo& relation_info, bool complete) const {
        std::cout << "Relation " << relation_info.relation()->id() << (complete ? "\n" : " (INCOMPLETE)\n");
        BOOST_FOREACH(const Osmium::OSM::Tag& tag, relation_info.relation()->tags()) {
            std::cout << "  " << tag.key() << "=" << tag.value() << "\n";
        }

        int i = 0;
        const Osmium::OSM::RelationMemberList& rml = relation_info.relation()->members();
        BOOST_FOREACH(const Osmium::OSM::RelationMember& rm, rml) {
            std::cout << "  [" << i << "] Member ";
            switch (rm.type()) {
                case 'n':
                    std::cout << "node";
                    break;
                case 'w':
                    std::cout << "way";
                    break;
                case 'r':
                    std::cout << "relation";
                    break;
            }
            std::cout << " " << rm.ref() << " with role '" << rm.role() << "'";
            if (relation_info.members()[i]) {
                std::cout << "\n";
                BOOST_FOREACH(const Osmium::OSM::Tag& tag, relation_info.members()[i]->tags()) {
                    std::cout << "      " << tag.key() << "=" << tag.value() << "\n";
                }
            } else {
                std::cout << " (NOT IN INPUT FILE)\n";
            }
            ++i;
        }
        std::cout << "\n";
    }

public:

    DebugRelationsAssembler() :
        AssemblerType() {
    }

    void relation(const shared_ptr<Osmium::OSM::Relation const>& relation) {
        add_relation(Osmium::Relations::RelationInfo(relation));
    }

    bool keep_member(Osmium::Relations::RelationInfo& /*relation_info*/, const Osmium::OSM::RelationMember& /*member*/) {
        return true;
    }

    void complete_relation(Osmium::Relations::RelationInfo& relation_info) {
        dump_relation(relation_info, true);
    }

    void all_members_available() {
        AssemblerType::clean_assembled_relations();
        BOOST_FOREACH(const Osmium::Relations::RelationInfo& relation_info, AssemblerType::relations()) {
            dump_relation(relation_info, false);
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

