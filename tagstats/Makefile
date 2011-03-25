#------------------------------------------------------------------------------
#
#  Osmium tagstats makefile
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

CXXFLAGS += -I../include -I../pbf

LDFLAGS = -L/usr/local/lib -lexpat -lpthread

LIB_SQLITE   = -lsqlite3
LIB_GD       = -lgd -lz -lm
LIB_PROTOBUF = -lz -lprotobuf

CPP = osmium.cpp XMLParser.cpp OsmMultipolygon.cpp
CPP_TAGSTAT = HandlerStatistics.cpp

OBJ = osmium.o XMLParser.o OsmMultipolygon.o
OBJ_TAGSTAT = HandlerStatistics.o
OBJ_PBF = fileformat.pb.o osmformat.pb.o

HPP = osmium.hpp Osm.hpp OsmObject.hpp OsmNode.hpp OsmWay.hpp OsmRelation.hpp OsmMultipolygon.hpp XMLParser.hpp wkb.hpp StringStore.hpp Handler.hpp HandlerBbox.hpp HandlerMultipolygon.hpp HandlerStatistics.hpp HandlerTagStats.hpp HandlerNodeLocationStore.hpp PBFParser.hpp

SRC_CPP = $(patsubst %,../src/%,$(CPP))
SRC_OBJ = $(patsubst %,../src/%,$(OBJ))
SRC_HPP = $(patsubst %,../include/%,$(HPP))

SRC_CPP_TAGSTAT = $(patsubst %,../src/%,$(CPP_TAGSTAT))
SRC_OBJ_TAGSTAT = $(patsubst %,../src/%,$(OBJ_TAGSTAT))

SRC_OBJ_PBF = $(patsubst %,../pbf/%,$(OBJ_PBF))

.PHONY: all clean doc protobuf

all: protobuf tagstats

protobuf:
	$(MAKE) -C ../pbf CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

%.o: ../src/%.cpp $(SRC_HPP)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

tagstats: tagstats.cpp $(OBJ) $(OBJ_TAGSTAT) $(SRC_HPP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OBJ) $(SRC_OBJ_PBF) $(OBJ_TAGSTAT) $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_GD) $(LIB_SQLITE) $(LIB_SHAPE)

install:
	install -m 755 -g root -o root tagstats $(DESTDIR)/usr/bin/tagstat
    
clean:
	rm -f *.o tagstats

