#------------------------------------------------------------------------------
#
#  Osmium osmjs makefile
#
#------------------------------------------------------------------------------

CXX = g++

CXXFLAGS = -g
#CXXFLAGS = -O3

CXXFLAGS += -std=c++0x -Wall -W -Wredundant-decls -Wdisabled-optimization -pedantic
#CXXFLAGS += -Wpadded -Winline

# uncomment this if you want information on how long it took to build the multipolygons
#CXXFLAGS += -DOSMIUM_WITH_MULTIPOLYGON_PROFILING

CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS += -DOSMIUM_WITH_GEOS $(shell geos-config --cflags)
CXXFLAGS += -DOSMIUM_WITH_SHPLIB
CXXFLAGS += -DOSMIUM_WITH_JAVASCRIPT

# Add this to force V8 garbage collection after each object/node/way/relation callback.
# Use only to find memory leaks. It will make osmjs really slow.
#CXXFLAGS += -DOSMIUM_V8_FORCE_GC

CXXFLAGS += -I../include -I../pbf

LDFLAGS = -L/usr/local/lib -lexpat -lpthread
LDFLAGS += $(shell geos-config --libs)

LIB_V8       = -lv8 -licuuc
LIB_SHAPE    = -lshp
LIB_PROTOBUF = -lz -lprotobuf

OBJ_PBF = fileformat.pb.o osmformat.pb.o
SRC_OBJ_PBF = $(patsubst %,../pbf/%,$(OBJ_PBF))

.PHONY: all clean doc protobuf install

all: protobuf osmjs

protobuf:
	$(MAKE) -C ../pbf CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

osmjs: osmjs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_V8) $(LIB_SHAPE)

install:
	install -m 755 -g root -o root osmjs $(DESTDIR)/usr/bin/osmjs
	install -m 755 -g root -o root -d $(DESTDIR)/usr/share/osmjs
	install -m 644 -g root -o root js/osm2shape.js $(DESTDIR)/usr/share/osmjs/osm2shape.js
    
clean:
	rm -f *.o core osmjs

