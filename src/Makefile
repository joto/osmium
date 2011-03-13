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

LIB_V8       = -lv8
LIB_SHAPE    = -lshp
LIB_PROTOBUF = -lz -lprotobuf

CPP = osmium.cpp XMLParser.cpp
CPP_JS = JavascriptTemplate.cpp

OBJ = osmium.o XMLParser.o
OBJ_JS = JavascriptTemplate.o
OBJ_PBF = fileformat.pb.o osmformat.pb.o

HPP = osmium.hpp Osm.hpp OsmObject.hpp OsmNode.hpp OsmWay.hpp OsmRelation.hpp OsmMultipolygon.hpp XMLParser.hpp wkb.hpp StringStore.hpp Handler.hpp HandlerBbox.hpp HandlerMultipolygon.hpp HandlerStatistics.hpp HandlerTagStats.hpp HandlerNodeLocationStore.hpp PBFParser.hpp

SRC_CPP = $(patsubst %,../src/%,$(CPP))
SRC_OBJ = $(patsubst %,../src/%,$(OBJ))
SRC_HPP = $(patsubst %,../include/%,$(HPP))

SRC_OBJ_PBF = $(patsubst %,../pbf/%,$(OBJ_PBF))

SRC_CPP_JS = $(patsubst %,../include/%,$(CPP_JS))
SRC_OBJ_JS = $(patsubst %,../include/%,$(OBJ_JS))
SRC_HPP_JS = $(patsubst %,../include/%,$(HPP_JS))
SRC_HPP_JS = $(wildcard ../include/*Javascript*.hpp)

.PHONY: all clean doc protobuf install

all: protobuf sizeof

protobuf:
	$(MAKE) -C ../pbf CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

%.o: ../src/%.cpp $(SRC_HPP)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

sizeof: sizeof.cpp $(OBJ) $(OBJ_JS) $(SRC_HPP) $(SRC_HPP_JS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJ) $(OBJ_JS) $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_V8) $(LIB_SHAPE)

install:
    
clean:
	rm -f *.o sizeof

