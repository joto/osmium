/*

  This is a small tool to show the contents of an input file
  containing serialized OSM format.

  The code in this example file is released into the Public Domain.

*/

#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <string>

#include <osmium.hpp>
#include <osmium/utils/timestamp.hpp>
#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/item.hpp>

void print_help() {
    std::cout << "osmium_serdebug [OPTIONS] INFILE\n" \
              << "\nOptions:\n" \
              << "  -h, --help       This help message\n" \
              << "  -s, --with-size  Report sizes of objects\n";
}

class Dump {

public:

    Dump(std::ostream& out, bool with_size=true, const std::string& prefix="") :
        m_out(out),
        m_with_size(with_size),
        m_prefix(prefix) {
    }

    void print_title(const char *title, const Osmium::Ser::TypedItem& item) const {
        m_out << m_prefix << title << ":";
        if (m_with_size) {
            m_out << " [" << item.size() << "]";
        }
        m_out << "\n";
    }

    void print_meta(const Osmium::Ser::Object& object) const {
        m_out << m_prefix << "  id="        << object.id() << "\n";
        m_out << m_prefix << "  version="   << object.version() << "\n";
        m_out << m_prefix << "  uid="       << object.uid() << "\n";
        m_out << m_prefix << "  user=|"     << object.user() << "|\n";
        m_out << m_prefix << "  changeset=" << object.changeset() << "\n";
        m_out << m_prefix << "  timestamp=" << Osmium::Timestamp::to_iso(object.timestamp()) << "\n";
        Dump dump(m_out, m_with_size, m_prefix + "  ");
        std::for_each(object.begin(), object.end(), dump);
    }

    void print_position(const Osmium::Ser::Node& node) const {
        const Osmium::OSM::Position& position = node.position();
        m_out << m_prefix << "  lon=" << std::fixed << std::setprecision(7) << position.lon() << "\n";
        m_out << m_prefix << "  lat=" << std::fixed << std::setprecision(7) << position.lat() << "\n";
    }

    void print_tag_list(const Osmium::Ser::TagList& tags) const {
        print_title("TAGS", tags);
        for (Osmium::Ser::TagList::iterator it = tags.begin(); it != tags.end(); ++it) {
            m_out << m_prefix << "  k=|" << it->key() << "| v=|" << it->value() << "|" << "\n";
        }
    }

    void print_way_node_list(const Osmium::Ser::WayNodeList& wnl) const {
        print_title("NODES", wnl);
        for (Osmium::Ser::WayNodeList::iterator it = wnl.begin(); it != wnl.end(); ++it) {
            m_out << m_prefix << "  ref=" << it->ref() << "\n";
        }
    }

    void print_way_node_list_with_position(const Osmium::Ser::WayNodeWithPositionList& wnl) const {
        print_title("NODES", wnl);
        for (Osmium::Ser::WayNodeWithPositionList::iterator it = wnl.begin(); it != wnl.end(); ++it) {
            m_out << m_prefix << "  ref=" << it->ref() << " pos=" << it->position() << "\n";
        }
    }
    
    void print_relation_member_list(const Osmium::Ser::RelationMemberList& rml) const {
        print_title("MEMBERS", rml);
        for (Osmium::Ser::RelationMemberList::iterator it = rml.begin(); it != rml.end(); ++it) {
            m_out << m_prefix << "  type=" << it->type() << " ref=" << it->ref() << " role=|" << it->role() << "|\n";
            if (it->full_member()) {
                Dump dump(m_out, m_with_size, m_prefix + "  | ");
                dump(it->get_object());
            }
        }
    }

    void operator()(const Osmium::Ser::TypedItem& item) const {
        switch (item.type().t()) {
            case Osmium::Ser::ItemType::itemtype_node:
                print_title("NODE", item);
                print_meta(static_cast<const Osmium::Ser::Object&>(item));
                print_position(static_cast<const Osmium::Ser::Node&>(item));
                break;
            case Osmium::Ser::ItemType::itemtype_way:
                print_title("WAY", item);
                print_meta(static_cast<const Osmium::Ser::Object&>(item));
                break;
            case Osmium::Ser::ItemType::itemtype_relation:
                print_title("RELATION", item);
                print_meta(static_cast<const Osmium::Ser::Object&>(item));
                break;
            case Osmium::Ser::ItemType::itemtype_tag_list:
                print_tag_list(static_cast<const Osmium::Ser::TagList&>(item));
                break;
            case Osmium::Ser::ItemType::itemtype_way_node_list:
                print_way_node_list(static_cast<const Osmium::Ser::WayNodeList&>(item));
                break;
            case Osmium::Ser::ItemType::itemtype_way_node_with_position_list:
                print_way_node_list_with_position(static_cast<const Osmium::Ser::WayNodeWithPositionList&>(item));
                break;
            case Osmium::Ser::ItemType::itemtype_relation_member_list:
            case Osmium::Ser::ItemType::itemtype_relation_member_list_with_full_members:
                print_relation_member_list(static_cast<const Osmium::Ser::RelationMemberList&>(item));
                break;
            default:
                print_title("UNKNOWN", item);
        }
    }

private:

    std::ostream& m_out;
    bool m_with_size;
    std::string m_prefix;

}; // class Dump

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);

    static struct option long_options[] = {
        {"help",      no_argument, 0, 'h'},
        {"with-size", no_argument, 0, 's'},
        {0, 0, 0, 0}
    };

    bool with_size = false;

    while (true) {
        int c = getopt_long(argc, argv, "hs", long_options, 0);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'h':
                print_help();
                exit(0);
            case 's':
                with_size = true;
                break;
            default:
                exit(1);
        }
    }

    int remaining_args = argc - optind;
    if (remaining_args != 1) {
        std::cerr << "Usage: " << argv[0] << " [OPTIONS] FILE\n";
        exit(1);
    }

    std::string infile(argv[optind]);

    typedef Osmium::Ser::BufferManager::FileInput manager_t;
    manager_t manager(infile);

    Dump dump(std::cout, with_size);
    std::for_each(manager.begin(), manager.end(), dump);
}

