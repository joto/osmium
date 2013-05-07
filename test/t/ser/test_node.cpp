#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/ser/item.hpp>
#include <osmium/handler/debug.hpp>

BOOST_AUTO_TEST_SUITE(SerNode)

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
    size_t bufsize = 10000;
    char* buffer = reinterpret_cast<char*>(malloc(bufsize));
    Osmium::Ser::Buffer out(buffer, bufsize);
    Osmium::Ser::Serializer ser(out);

    Osmium::OSM::Node n1;
    n1.id(10);
    n1.version(1);
    n1.changeset(21);
    n1.uid(17);
    n1.timestamp(time(NULL));
    n1.lon(10);
    n1.lat(20);
    n1.tags().add("amenity", "restaurant");
    n1.tags().add("foo", "bar");
    ser.add_node(n1);

    Osmium::OSM::Node n2;
    n2.id(11);
    n2.version(1);
    n2.changeset(22);
    n2.uid(18);
    n2.timestamp(time(NULL));
    n2.lon(11);
    n2.lat(22);
    ser.add_node(n2);

    Osmium::Ser::Deserializer<Osmium::Handler::Debug> deser(out);
    Osmium::Handler::Debug debug;
    deser.feed(debug);
}

BOOST_AUTO_TEST_SUITE_END()
