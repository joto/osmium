
# README-ser.md

This explains some of the serialization work in this, the "ser" branch. Note
that all of this is currently rather experimental and it *will* change in the
future.

Most of the new code is in the "include/osmium/ser directory". It allows you
to serialize OSM objects into a memory buffer (and write them to a file from
there) and to read OSM objects back from this serialized form. There are also
ways to create indexes to quickly find objects in those data dump files again
and to create and query mappings to find out which ways or relations contain a
given node, way, or relation as member.

To try it out, there are three commands in the "examples" directory:

## osmium_serdump

This will read any kind of OSM file and serialize the contents to disk. You
have to call it with the name of the OSM file and the name of the target
directory. The target directory will be created if it doesn't exist:

  osmium_serdump foo.osm.pbf dumpdir

Several files will be created in the "dumpdir" directory:

* data.osm.ser               - contains the OSM data in binary format
* (nodes|ways|relations).idx - index files for looking up the position
                               of an object in the data file by it's id
* node2way.map               - contains mapping from node ID to IDs of
                               all ways containing this node
* (node|way|rel)2rel.map     - contains mapping from (node|way|relation)
                               ID to IDs of all relations containing this
                               object

## osmium_serdebug

Dumps the contents of the data file in a readable format to stdout. It is
called with the name of the data directory created with osmium_serdump:

  osmium_serdebug dumpdir

or with the name of the data file:

  osmium_serdebug dumpdir/data.osm.ser

## osmium_serget

This programm does lookups using the index and map files in the data directory.
Parameters are the name of the data directory, the lookup type and an ID. Here
are some examples:

  osmium_serget dumpdir n 17

will look up node ID 17 in the nodes.idx file and show you the data of this
node from the data file in the same format that osmium_serdebug uses.

To look up ways or relations instead, use "w" or "r":

  osmium_serget dumpdir w 123
  osmium_serget dumpdir r 456

To look up something using the map files use the following syntax:

  osmium_serget dumpdir n2w 89

This will lookup up node ID 89 and tell you the IDs of all ways this node is in.
Similar for "n2r", "w2r", and "r2r" to find out which relations a node, way, or
relation, is a member of.

If there was nothing found, the program will exit with return code 1 and
without any output.

