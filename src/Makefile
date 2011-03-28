#------------------------------------------------------------------------------
#
#  Osmium makefile
#
#------------------------------------------------------------------------------

CXX = g++

CXXFLAGS = -g
#CXXFLAGS = -O3

CXXFLAGS += -std=c++0x -Wall -W -Wredundant-decls -Wdisabled-optimization -pedantic
#CXXFLAGS += -Wpadded -Winline

# uncomment this if you want information on how long it took to build the multipolygons
#CXXFLAGS += -DWITH_MULTIPOLYGON_PROFILING

CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS += -DWITH_GEOS $(shell geos-config --cflags)
CXXFLAGS += -DWITH_SHPLIB

CXXFLAGS += -I../include -I../pbf

LDFLAGS = -L/usr/local/lib -lexpat -lpthread
LDFLAGS += $(shell geos-config --libs)

LIB_SQLITE   = -lsqlite3
LIB_V8       = -lv8
LIB_SHAPE    = -lshp
LIB_PROTOBUF = -lz -lprotobuf

CPP = osmium.cpp XMLParser.cpp

OBJ = osmium.o XMLParser.o
OBJ_PBF = fileformat.pb.o osmformat.pb.o

HPP = osmium.hpp Osm.hpp OsmObject.hpp OsmNode.hpp OsmWay.hpp OsmRelation.hpp OsmMultipolygon.hpp XMLParser.hpp wkb.hpp StringStore.hpp Handler.hpp HandlerBbox.hpp HandlerMultipolygon.hpp HandlerStatistics.hpp HandlerTagStats.hpp HandlerNodeLocationStore.hpp PBFParser.hpp

SRC_CPP = $(patsubst %,../src/%,$(CPP))
SRC_OBJ = $(patsubst %,../src/%,$(OBJ))
SRC_HPP = $(patsubst %,../include/%,$(HPP))

SRC_OBJ_PBF = $(patsubst %,../pbf/%,$(OBJ_PBF))

.PHONY: all clean doc protobuf install

all: protobuf osmium_sizeof osmium_debug osmium_stats

protobuf:
	$(MAKE) -C ../pbf CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

%.o: ../src/%.cpp $(SRC_HPP)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

osmium_sizeof: osmium_sizeof.cpp $(OBJ) $(SRC_HPP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJ) $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_V8) $(LIB_SHAPE)

osmium_debug: osmium_debug.cpp $(OBJ) $(SRC_HPP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJ) $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_V8) $(LIB_SHAPE)

osmium_stats: osmium_stats.cpp $(OBJ) $(SRC_HPP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJ) $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_SQLITE) $(LIB_PROTOBUF) $(LIB_SHAPE)

install:
    
clean:
	rm -f *.o core osmium_sizeof osmium_debug osmium_stats

