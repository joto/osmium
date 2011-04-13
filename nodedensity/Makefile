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

CXXFLAGS += -I../include

LDFLAGS = -L/usr/local/lib -lexpat -lpthread

LIB_GD       = -lgd -lz -lm
LIB_PROTOBUF = -lz -lprotobuf -losmpbf

.PHONY: all clean install

all: nodedensity

nodedensity: nodedensity.cpp handler_nodedensity.hpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_PROTOBUF) $(LIB_GD)

install:
	install -m 755 -g root -o root -d $(DESTDIR)/usr/bin
	install -m 755 -g root -o root nodedensity $(DESTDIR)/usr/bin/nodedensity
    
clean:
	rm -f *.o core nodedensity

