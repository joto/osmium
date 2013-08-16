#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <osmium/handler.hpp>
#include <osmium/handler/debug.hpp>

BOOST_AUTO_TEST_SUITE(Handler_Forward)

BOOST_AUTO_TEST_CASE(ForwardHandler_methods_forwardAllHandlerCalls) {
    // we use a debug handler to check the correct order of method calls
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    // this is the object under test, let it forward all calls to the debugHandler
    Osmium::Handler::Forward<Osmium::Handler::Debug> forwardHandler(debugHandler);

    // test all handler calls
    Osmium::OSM::Meta meta;
    forwardHandler.init(meta);

    forwardHandler.before_nodes();
    shared_ptr<Osmium::OSM::Node> node_ptr = make_shared<Osmium::OSM::Node>();
    forwardHandler.node(node_ptr);
    forwardHandler.after_nodes();

    forwardHandler.before_ways();
    shared_ptr<Osmium::OSM::Way> way_ptr = make_shared<Osmium::OSM::Way>();
    forwardHandler.way(way_ptr);
    forwardHandler.after_ways();

    forwardHandler.before_relations();
    shared_ptr<Osmium::OSM::Relation> rel_ptr = make_shared<Osmium::OSM::Relation>();
    forwardHandler.relation(rel_ptr);
    forwardHandler.after_relations();

    forwardHandler.final();

    BOOST_CHECK(output.is_equal("\
meta:\n\
  generator=\n\
before_nodes\n\
node:\n\
  id=0\n\
  version=0\n\
  uid=-1\n\
  user=||\n\
  changeset=0\n\
  timestamp=\n\
  tags: (count=0)\n\
  lon=214.7483647\n\
  lat=214.7483647\n\
after_nodes\n\
before_ways\n\
way:\n\
  id=0\n\
  version=0\n\
  uid=-1\n\
  user=||\n\
  changeset=0\n\
  timestamp=\n\
  tags: (count=0)\n\
  node_count=0\n\
  nodes:\n\
after_ways\n\
before_relations\n\
relation:\n\
  id=0\n\
  version=0\n\
  uid=-1\n\
  user=||\n\
  changeset=0\n\
  timestamp=\n\
  tags: (count=0)\n\
  members: (count=0)\n\
after_relations\n\
final\n\
"));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Handler_Sequence)

BOOST_AUTO_TEST_CASE(SequenceHandler_methods_forwardAllHandlerCallsInSequence) {
    // we use two debug handler to check the correct order of method calls
    boost::test_tools::output_test_stream output1, output2;
    Osmium::Handler::Debug debugHandler1(false, output1);
    Osmium::Handler::Debug debugHandler2(false, output2);

    // this is the object under test, let it forward all calls to the debugHandler
    Osmium::Handler::Sequence<Osmium::Handler::Debug,Osmium::Handler::Debug> sequenceHandler(debugHandler1, debugHandler2);

    // test all handler calls
    Osmium::OSM::Meta meta;
    sequenceHandler.init(meta);

    sequenceHandler.before_nodes();
    shared_ptr<Osmium::OSM::Node> node_ptr = make_shared<Osmium::OSM::Node>();
    sequenceHandler.node(node_ptr);
    sequenceHandler.after_nodes();

    sequenceHandler.before_ways();
    shared_ptr<Osmium::OSM::Way> way_ptr = make_shared<Osmium::OSM::Way>();
    sequenceHandler.way(way_ptr);
    sequenceHandler.after_ways();

    sequenceHandler.before_relations();
    shared_ptr<Osmium::OSM::Relation> rel_ptr = make_shared<Osmium::OSM::Relation>();
    sequenceHandler.relation(rel_ptr);
    sequenceHandler.after_relations();

    sequenceHandler.final();

    BOOST_CHECK(output1.is_equal("\
meta:\n\
  generator=\n\
before_nodes\n\
node:\n\
  id=0\n\
  version=0\n\
  uid=-1\n\
  user=||\n\
  changeset=0\n\
  timestamp=\n\
  tags: (count=0)\n\
  lon=214.7483647\n\
  lat=214.7483647\n\
after_nodes\n\
before_ways\n\
way:\n\
  id=0\n\
  version=0\n\
  uid=-1\n\
  user=||\n\
  changeset=0\n\
  timestamp=\n\
  tags: (count=0)\n\
  node_count=0\n\
  nodes:\n\
after_ways\n\
before_relations\n\
relation:\n\
  id=0\n\
  version=0\n\
  uid=-1\n\
  user=||\n\
  changeset=0\n\
  timestamp=\n\
  tags: (count=0)\n\
  members: (count=0)\n\
after_relations\n\
final\n\
"));
    BOOST_CHECK(output2.is_equal("\
meta:\n\
  generator=\n\
before_nodes\n\
node:\n\
  id=0\n\
  version=0\n\
  uid=-1\n\
  user=||\n\
  changeset=0\n\
  timestamp=\n\
  tags: (count=0)\n\
  lon=214.7483647\n\
  lat=214.7483647\n\
after_nodes\n\
before_ways\n\
way:\n\
  id=0\n\
  version=0\n\
  uid=-1\n\
  user=||\n\
  changeset=0\n\
  timestamp=\n\
  tags: (count=0)\n\
  node_count=0\n\
  nodes:\n\
after_ways\n\
before_relations\n\
relation:\n\
  id=0\n\
  version=0\n\
  uid=-1\n\
  user=||\n\
  changeset=0\n\
  timestamp=\n\
  tags: (count=0)\n\
  members: (count=0)\n\
after_relations\n\
final\n\
"));
}

BOOST_AUTO_TEST_SUITE_END()
