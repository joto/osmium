#------------------------------------------------------------------------------
#
#  Makefile for Osmium examples.
#
#------------------------------------------------------------------------------

CXX = g++
#CXX = clang++

CXXFLAGS = -g
#CXXFLAGS = -O3 -g

CXXFLAGS += -Wall -Wextra -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long

# uncomment this if you want information on how long it took to build the multipolygons
#CXXFLAGS += -DOSMIUM_WITH_MULTIPOLYGON_PROFILING

CXXFLAGS_GEOS    = $(shell geos-config --cflags)
CXXFLAGS_LIBXML2 = $(shell xml2-config --cflags)
CXXFLAGS_OGR     = $(shell gdal-config --cflags)

CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS += -I../include

# remove this if you do not want debugging to be compiled in
CXXFLAGS += -DOSMIUM_WITH_DEBUG

LIB_EXPAT  = -lexpat
LIB_GD     = -lgd -lz -lm
LIB_GEOS   = $(shell geos-config --libs)
LIB_OGR    = $(shell gdal-config --libs)
LIB_PBF    = -lz -lpthread -lprotobuf-lite -losmpbf
LIB_SHAPE  = -lshp $(LIB_GEOS)
LIB_SQLITE = -lsqlite3
LIB_XML2   = $(shell xml2-config --libs)

LDFLAGS = $(LIB_EXPAT) $(LIB_PBF)

PROGRAMS = \
    osmium_convert \
    osmium_debug \
    osmium_find_bbox \
    osmium_mpdump \
    osmium_progress \
    osmium_range_from_history \
    osmium_relation_members \
    osmium_sizeof \
    osmium_stats \
    osmium_store_and_debug \
    osmium_time \
    osmium_toogr \
    osmium_toogr2 \
    osmium_toshape \
    nodedensity

.PHONY: all clean

all: $(PROGRAMS)

osmium_convert: osmium_convert.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_LIBXML2) -o $@ $< $(LDFLAGS) $(LIB_XML2)

osmium_debug: osmium_debug.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
#	$(CXX) $(CXXFLAGS) -DOSMIUM_DEBUG_WITH_ENDTIME -o $@ $< $(LDFLAGS)

osmium_find_bbox: osmium_find_bbox.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

osmium_mpdump: osmium_mpdump.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_OGR) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_OGR) $(LIB_GEOS)

osmium_progress: osmium_progress.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

osmium_range_from_history: osmium_range_from_history.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_LIBXML2) -o $@ $< $(LDFLAGS) $(LIB_XML2)

osmium_relation_members: osmium_relation_members.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

osmium_sizeof: osmium_sizeof.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

osmium_stats: osmium_stats.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_SQLITE)

osmium_store_and_debug: osmium_store_and_debug.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

osmium_time: osmium_time.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

osmium_toogr: osmium_toogr.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_OGR)

osmium_toogr2: osmium_toogr2.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_OGR) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_OGR) $(LIB_GEOS)

osmium_toshape: osmium_toshape.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_SHAPE)

nodedensity: nodedensity.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_GD)

clean:
	rm -f *.o core $(PROGRAMS)

