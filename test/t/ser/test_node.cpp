#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/ser/buffer_manager.hpp>
#include <osmium/ser/deserializer.hpp>
#include <osmium/ser/index.hpp>
#include <osmium/ser/handler.hpp>
#include <osmium/handler/debug.hpp>
#include <osmium/smart_ptr.hpp>

class TestHandler : public Osmium::Handler::Base {

    public:

        TestHandler() : Base(), m_count(0) {
        }

        void node(const shared_ptr<Osmium::OSM::Node const>& node) {
            switch (m_count) {
                case 0:
                    BOOST_CHECK_EQUAL(node->id(), 10);
                    BOOST_CHECK_EQUAL(node->version(), 1);
                    BOOST_CHECK_EQUAL(node->changeset(), 21);
                    BOOST_CHECK_EQUAL(node->uid(), 17);
                    BOOST_CHECK_EQUAL(node->lon(), 10);
                    BOOST_CHECK_EQUAL(node->lat(), 20);
                    BOOST_CHECK_EQUAL(node->tags().size(), 2);
                    break;
                case 1:
                    BOOST_CHECK_EQUAL(node->id(), 11);
                    BOOST_CHECK_EQUAL(node->version(), 1);
                    BOOST_CHECK_EQUAL(node->changeset(), 22);
                    BOOST_CHECK_EQUAL(node->uid(), 18);
                    BOOST_CHECK_EQUAL(node->lon(), 11);
                    BOOST_CHECK_EQUAL(node->lat(), 22);
                    BOOST_CHECK_EQUAL(node->lat(), 22);
                    BOOST_CHECK_EQUAL(node->tags().size(), 0);
                    break;
            }
            ++m_count;
        }

    private:

        int m_count;

}; // class TestHandler

BOOST_AUTO_TEST_SUITE(SerNode)

BOOST_AUTO_TEST_CASE(ser_deser) {
    const size_t buffer_size = 10000;
    Osmium::Ser::BufferManager::Malloc manager(buffer_size);
    Osmium::Ser::Index::Null fake_index;
    Osmium::Ser::Handler<Osmium::Ser::BufferManager::Malloc, Osmium::Ser::Index::Null, Osmium::Ser::Index::Null, Osmium::Ser::Index::Null> handler(manager, fake_index, fake_index, fake_index);

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

    Osmium::Ser::Buffer& out = manager.buffer();
    BOOST_CHECK_EQUAL(out.size(), buffer_size);
    BOOST_CHECK_EQUAL(out.committed(), 176);

    Osmium::Ser::Deserializer<TestHandler> deser(out);
    TestHandler test_handler;
    deser.feed(test_handler);
}

BOOST_AUTO_TEST_SUITE_END()
