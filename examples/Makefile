#------------------------------------------------------------------------------
#
#  Makefile for Osmium examples.
#
#------------------------------------------------------------------------------

CXX = g++

CXXFLAGS = -g
#CXXFLAGS = -O3

CXXFLAGS += -Wall -Wextra -Wredundant-decls -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo
#CXXFLAGS += -Wpadded -Winline -Weffc++ -Wold-style-cast

# uncomment this if you want information on how long it took to build the multipolygons
#CXXFLAGS += -DOSMIUM_WITH_MULTIPOLYGON_PROFILING

CXXFLAGS_SHAPE   = -DOSMIUM_WITH_SHPLIB -DOSMIUM_WITH_GEOS $(shell geos-config --cflags)
CXXFLAGS_LIBXML2 = -DOSMIUM_WITH_OUTPUT_OSM_XML $(shell xml2-config --cflags)
CXXFLAGS_OGR     = -DOSMIUM_WITH_OGR $(shell gdal-config --cflags)

CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

CXXFLAGS += -I../include

LDFLAGS = -L/usr/local/lib -lexpat -lpthread

LIB_GD       = -lgd -lz -lm
LIB_SQLITE   = -lsqlite3
LIB_V8       = -lv8
LIB_GEOS     = $(shell geos-config --libs)
LIB_SHAPE    = -lshp $(LIB_GEOS)
LIB_PROTOBUF = -lz -lprotobuf-lite -losmpbf
LIB_XML2     = $(shell xml2-config --libs)
LIB_OGR      = $(shell gdal-config --libs)

PROGRAMS = osmium_sizeof osmium_debug osmium_stats osmium_find_bbox osmium_time osmium_progress osmium_convert osmium_toogr osmium_toshape nodedensity

.PHONY: all clean

all: $(PROGRAMS)

osmium_sizeof: osmium_sizeof.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF)

osmium_debug: osmium_debug.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF)

osmium_toshape: osmium_toshape.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_SHAPE) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_SHAPE)

osmium_toogr: osmium_toogr.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_OGR) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_OGR)

osmium_stats: osmium_stats.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_SQLITE)

osmium_find_bbox: osmium_find_bbox.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF)

osmium_time: osmium_time.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF)

osmium_progress: osmium_progress.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF)

osmium_convert: osmium_convert.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_LIBXML2) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_XML2)

osmium_poly_reader: osmium_poly_reader.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_GEOS) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_GEOS)

nodedensity: nodedensity.cpp handler_nodedensity.hpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_GD)

clean:
	rm -f *.o core $(PROGRAMS)

