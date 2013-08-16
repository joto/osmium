#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <osmium/handler/debug.hpp>

BOOST_AUTO_TEST_SUITE(Handler_Debug)

BOOST_AUTO_TEST_CASE(Debug_init_showsMetadata) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    Osmium::OSM::Meta meta;
    meta.generator("unit tests");

    debugHandler.init(meta);
    BOOST_CHECK(output.is_equal("meta:\n  generator=unit tests\n"));
}

BOOST_AUTO_TEST_CASE(Debug_init_showsBounds) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    Osmium::OSM::Meta meta;
    meta.generator("unit tests");
    meta.bounds().extend(Osmium::OSM::Position(12000000, 45000000));
    meta.bounds().extend(Osmium::OSM::Position(13000000, 46000000));

    debugHandler.init(meta);
    BOOST_CHECK(output.is_equal("meta:\n  generator=unit tests\n  bounds=(1.2,4.5,1.3,4.6)\n"));
}

BOOST_AUTO_TEST_CASE(Debug_beforeNodes_showsMessage) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    debugHandler.before_nodes();
    BOOST_CHECK(output.is_equal("before_nodes\n"));
}

BOOST_AUTO_TEST_CASE(Debug_afterNodes_showsMessage) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    debugHandler.after_nodes();
    BOOST_CHECK(output.is_equal("after_nodes\n"));
}

BOOST_AUTO_TEST_CASE(Debug_beforeWays_showsMessage) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    debugHandler.before_ways();
    BOOST_CHECK(output.is_equal("before_ways\n"));
}

BOOST_AUTO_TEST_CASE(Debug_afterWays_showsMessage) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    debugHandler.after_ways();
    BOOST_CHECK(output.is_equal("after_ways\n"));
}

BOOST_AUTO_TEST_CASE(Debug_beforeRelations_showsMessage) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    debugHandler.before_relations();
    BOOST_CHECK(output.is_equal("before_relations\n"));
}

BOOST_AUTO_TEST_CASE(Debug_afterRelations_showsMessage) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    debugHandler.after_relations();
    BOOST_CHECK(output.is_equal("after_relations\n"));
}

BOOST_AUTO_TEST_CASE(Debug_final_showsMessage) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    debugHandler.final();
    BOOST_CHECK(output.is_equal("final\n"));
}

BOOST_AUTO_TEST_CASE(Debug_node_showsNodeData) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    shared_ptr<Osmium::OSM::Node> node_ptr = make_shared<Osmium::OSM::Node>();

    node_ptr->id(12);
    node_ptr->version(1u);
    node_ptr->uid(13);
    node_ptr->user("L33t User");
    node_ptr->visible(true);
    node_ptr->changeset(14);
    node_ptr->timestamp((time_t)1362135600u);
    node_ptr->position(Osmium::OSM::Position(12000000, 45000000));

    debugHandler.node(node_ptr);
    BOOST_CHECK(output.is_equal("\
node:\n\
  id=12\n\
  version=1\n\
  uid=13\n\
  user=|L33t User|\n\
  changeset=14\n\
  timestamp=2013-03-01T11:00:00Z\n\
  tags: (count=0)\n\
  lon=1.2000000\n\
  lat=4.5000000\n\
"));
}

BOOST_AUTO_TEST_CASE(Debug_node_showsTagList) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    shared_ptr<Osmium::OSM::Node> node_ptr = make_shared<Osmium::OSM::Node>();

    node_ptr->id(12);
    node_ptr->version(1u);
    node_ptr->uid(13);
    node_ptr->user("L33t User");
    node_ptr->visible(true);
    node_ptr->changeset(14);
    node_ptr->timestamp((time_t)1362135600u);
    node_ptr->position(Osmium::OSM::Position(12000000, 45000000));
    
    node_ptr->tags().add("example", "one");
    node_ptr->tags().add("example2", "two");

    debugHandler.node(node_ptr);
    BOOST_CHECK(output.is_equal("\
node:\n\
  id=12\n\
  version=1\n\
  uid=13\n\
  user=|L33t User|\n\
  changeset=14\n\
  timestamp=2013-03-01T11:00:00Z\n\
  tags: (count=2)\n\
    k=|example| v=|one|\n\
    k=|example2| v=|two|\n\
  lon=1.2000000\n\
  lat=4.5000000\n\
"));
}

BOOST_AUTO_TEST_CASE(Debug_way_showsWayData) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    shared_ptr<Osmium::OSM::Way> way_ptr = make_shared<Osmium::OSM::Way>();

    way_ptr->id(12);
    way_ptr->version(1u);
    way_ptr->uid(13);
    way_ptr->user("L33t User");
    way_ptr->visible(true);
    way_ptr->changeset(14);
    way_ptr->timestamp((time_t)1362135600u);

    way_ptr->nodes().add(1);
    way_ptr->nodes().add(2);
    way_ptr->nodes().add(3);

    debugHandler.way(way_ptr);

    BOOST_CHECK(output.is_equal("\
way:\n\
  id=12\n\
  version=1\n\
  uid=13\n\
  user=|L33t User|\n\
  changeset=14\n\
  timestamp=2013-03-01T11:00:00Z\n\
  tags: (count=0)\n\
  node_count=3\n\
  nodes:\n\
    ref=1\n\
    ref=2\n\
    ref=3\n\
"));
}

BOOST_AUTO_TEST_CASE(Debug_relation_showsRelationData) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    shared_ptr<Osmium::OSM::Relation> rel_ptr = make_shared<Osmium::OSM::Relation>();

    rel_ptr->id(12);
    rel_ptr->version(1u);
    rel_ptr->uid(13);
    rel_ptr->user("L33t User");
    rel_ptr->visible(true);
    rel_ptr->changeset(14);
    rel_ptr->timestamp((time_t)1362135600u);

    rel_ptr->add_member('n', 1, "role1");
    rel_ptr->add_member('w', 2, "role2");
    rel_ptr->add_member('r', 3, "role3");

    debugHandler.relation(rel_ptr);

    BOOST_CHECK(output.is_equal("\
relation:\n\
  id=12\n\
  version=1\n\
  uid=13\n\
  user=|L33t User|\n\
  changeset=14\n\
  timestamp=2013-03-01T11:00:00Z\n\
  tags: (count=0)\n\
  members: (count=3)\n\
    type=n ref=1 role=|role1|\n\
    type=w ref=2 role=|role2|\n\
    type=r ref=3 role=|role3|\n\
"));
}

BOOST_AUTO_TEST_CASE(Debug_node_showsVisibilityInformationIfStreamHasMultipleObjectVersions) {
    boost::test_tools::output_test_stream output;
    Osmium::Handler::Debug debugHandler(false, output);

    shared_ptr<Osmium::OSM::Node> node_ptr = make_shared<Osmium::OSM::Node>();

    node_ptr->id(12);
    node_ptr->version(1u);
    node_ptr->uid(13);
    node_ptr->user("L33t User");
    node_ptr->visible(false);
    node_ptr->changeset(14);
    node_ptr->timestamp((time_t)1362135600u);
    node_ptr->endtime((time_t)1362135600u);
    node_ptr->position(Osmium::OSM::Position(12000000, 45000000));

    Osmium::OSM::Meta meta;
    meta.generator("unit tests");
    meta.has_multiple_object_versions(true);
    debugHandler.init(meta);

    debugHandler.node(node_ptr);
    BOOST_CHECK(output.is_equal("\
meta:\n\
  generator=unit tests\n\
node:\n\
  id=12\n\
  version=1\n\
  uid=13\n\
  user=|L33t User|\n\
  changeset=14\n\
  timestamp=2013-03-01T11:00:00Z\n\
  visible=no\n\
  endtime=2013-03-01T11:00:00Z\n\
  tags: (count=0)\n\
  lon=1.2000000\n\
  lat=4.5000000\n\
"));
}
BOOST_AUTO_TEST_SUITE_END()
