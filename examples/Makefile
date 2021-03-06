#------------------------------------------------------------------------------
#
#  Makefile for Osmium examples
#
#------------------------------------------------------------------------------
#
#  You can set several environment variables before running make if you don't
#  like the defaults:
#
#  CXX                - Your C++ compiler.
#  CPLUS_INCLUDE_PATH - Include file search path.
#  CXXFLAGS           - Extra compiler flags.
#  LDFLAGS            - Extra linker flags.
#  
#------------------------------------------------------------------------------

CXXFLAGS += -O3
#CXXFLAGS += -g
CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS += -I../include -I/usr/include/libshp

# remove this if you do not want debugging to be compiled in
CXXFLAGS += -DOSMIUM_WITH_DEBUG

CXXFLAGS_GEOS     := $(shell geos-config --cflags)
CXXFLAGS_LIBXML2  := $(shell xml2-config --cflags)
CXXFLAGS_OGR      := $(shell gdal-config --cflags)
CXXFLAGS_WARNINGS := -Wall -Wextra -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long

LIB_EXPAT  := -lexpat
LIB_PBF    := -lz -lpthread -lprotobuf-lite -losmpbf
LIB_GD     := -lgd -lz -lm
LIB_GEOS   := $(shell geos-config --libs)
LIB_OGR    := $(shell gdal-config --libs)
LIB_SHAPE  := -lshp $(LIB_GEOS)
LIB_XML2   := $(shell xml2-config --libs)

PROGRAMS := \
    osmium_convert \
    osmium_debug \
    osmium_find_bbox \
    osmium_mpdump \
    osmium_progress \
    osmium_range_from_history \
    osmium_relation_members \
    osmium_sizeof \
    osmium_store_and_debug \
    osmium_time \
    osmium_toogr \
    osmium_toogr2 \
    osmium_to_postgis \
    osmium_toshape \
    nodedensity

.PHONY: all clean

all: $(PROGRAMS)

osmium_convert: osmium_convert.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_LIBXML2) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_XML2)

osmium_debug: osmium_debug.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)
#	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -DOSMIUM_DEBUG_WITH_ENDTIME -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_find_bbox: osmium_find_bbox.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_mpdump: osmium_mpdump.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_GEOS)

osmium_progress: osmium_progress.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_range_from_history: osmium_range_from_history.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_LIBXML2) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_XML2)

osmium_relation_members: osmium_relation_members.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_sizeof: osmium_sizeof.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_store_and_debug: osmium_store_and_debug.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_time: osmium_time.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF)

osmium_toogr: osmium_toogr.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_OGR)

osmium_toogr2: osmium_toogr2.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_OGR) $(LIB_GEOS)

osmium_to_postgis: osmium_to_postgis.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_OGR)

osmium_toshape: osmium_toshape.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_SHAPE)

nodedensity: nodedensity.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_GD)

clean:
	rm -f *.o core $(PROGRAMS)

