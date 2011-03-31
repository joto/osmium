#------------------------------------------------------------------------------
#
#  Osmium nodedensity makefile
#
#------------------------------------------------------------------------------

CXX = g++

CXXFLAGS = -g
#CXXFLAGS = -O3

CXXFLAGS += -std=c++0x -Wall -W -Wredundant-decls -Wdisabled-optimization -pedantic
#CXXFLAGS += -Wpadded -Winline

CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

CXXFLAGS += -I../include -I../pbf

LDFLAGS = -L/usr/local/lib -lexpat -lpthread

LIB_GD       = -lgd -lz -lm
LIB_PROTOBUF = -lz -lprotobuf

OBJ_PBF = fileformat.pb.o osmformat.pb.o

SRC_OBJ_PBF = $(patsubst %,../pbf/%,$(OBJ_PBF))

.PHONY: all clean doc protobuf

all: protobuf nodedensity

protobuf:
	$(MAKE) -C ../pbf CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

%.o: ../src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

nodedensity: nodedensity.cpp handler_nodedensity.hpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC_OBJ_PBF) $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_GD)

install:
	install -m 755 -g root -o root nodedensity $(DESTDIR)/usr/bin/nodedensity
    
clean:
	rm -f *.o core nodedensity

