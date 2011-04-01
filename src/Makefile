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

CXXFLAGS += -I../include -I../pbf

LDFLAGS = -L/usr/local/lib -lexpat -lpthread

LIB_SQLITE   = -lsqlite3
LIB_V8       = -lv8
LIB_SHAPE    = -lshp
LIB_PROTOBUF = -lz -lprotobuf

OBJ_PBF = fileformat.pb.o osmformat.pb.o

SRC_OBJ_PBF = $(patsubst %,../pbf/%,$(OBJ_PBF))

.PHONY: all clean doc protobuf install

all: protobuf osmium_sizeof osmium_debug osmium_stats

protobuf:
	$(MAKE) -C ../pbf CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

osmium_sizeof: osmium_sizeof.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_PROTOBUF)

osmium_debug: osmium_debug.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_PROTOBUF)

osmium_stats: osmium_stats.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_SQLITE) $(LIB_PROTOBUF)

install:
    
clean:
	rm -f *.o core osmium_sizeof osmium_debug osmium_stats

