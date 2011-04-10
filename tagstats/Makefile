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
#CXXFLAGS += -DOSMIUM_WITH_MULTIPOLYGON_PROFILING

CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

CXXFLAGS += -I../include -I../pbf

LDFLAGS = -L/usr/local/lib -lexpat -lpthread

LIB_SQLITE   = -lsqlite3
LIB_GD       = -lgd -lz -lm
LIB_PROTOBUF = -lz -lprotobuf

OBJ_PBF = fileformat.pb.o osmformat.pb.o

SRC_OBJ_PBF = $(patsubst %,../pbf/%,$(OBJ_PBF))

.PHONY: all clean doc protobuf

all: protobuf tagstats

protobuf:
	$(MAKE) -C ../pbf CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

tagstats: tagstats.cpp handler_tagstats.hpp string_store.hpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_GD) $(LIB_SQLITE)

install:
	install -m 755 -g root -o root tagstats $(DESTDIR)/usr/bin/tagstat
    
clean:
	rm -f *.o core tagstats

