#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/ser/item.hpp>
#include <osmium/handler/debug.hpp>

BOOST_AUTO_TEST_SUITE(SerNode)

BOOST_AUTO_TEST_CASE(instantiation_with_default_parameters) {
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

    size_t bufsize = 10000;
    char* buffer = reinterpret_cast<char*>(malloc(bufsize));
    Osmium::Ser::Buffer out(buffer, bufsize);
    Osmium::Ser::Serializer ser(out);
    uint64_t size = ser.add_node(n1);
    BOOST_CHECK_EQUAL(0, size); // wrong

    Osmium::Ser::Deserializer<Osmium::Handler::Debug> deser(out);
    Osmium::Handler::Debug debug;
    deser.feed(debug);
}

BOOST_AUTO_TEST_SUITE_END()
