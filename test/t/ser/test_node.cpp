#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/deserializer.hpp>
#include <osmium/ser/handler.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/smart_ptr.hpp>

BOOST_AUTO_TEST_SUITE(SerNode)

BOOST_AUTO_TEST_CASE(ser_deser) {
    Osmium::Ser::BufferManager::Malloc manager(10000);
    Osmium::Ser::Handler<Osmium::Ser::BufferManager::Malloc> handler(manager);

    shared_ptr<Osmium::OSM::Node> n1 = make_shared<Osmium::OSM::Node>();
    n1->id(10);
    n1->version(1);
    n1->changeset(21);
    n1->uid(17);
    n1->timestamp(time(NULL));
    n1->lon(10);
    n1->lat(20);
    n1->tags().add("amenity", "restaurant");
    n1->tags().add("foo", "bar");
    handler.node(n1);

    shared_ptr<Osmium::OSM::Node> n2 = make_shared<Osmium::OSM::Node>();
    n2->id(11);
    n2->version(1);
    n2->changeset(22);
    n2->uid(18);
    n2->timestamp(time(NULL));
    n2->lon(11);
    n2->lat(22);
    handler.node(n2);

    handler.final();

    Osmium::Ser::Buffer& out = manager.buffer();
    Osmium::Ser::Deserializer<Osmium::Handler::Debug> deser(out);
    Osmium::Handler::Debug debug;
    deser.feed(debug);
}

BOOST_AUTO_TEST_SUITE_END()
