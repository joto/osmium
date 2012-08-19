#------------------------------------------------------------------------------
#
#  Osmium osmjs makefile
#
#------------------------------------------------------------------------------

CXX = g++
#CXX = clang++

CXXFLAGS = -g
#CXXFLAGS = -O3 -g

CXXFLAGS += -Wall -Wextra -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long

CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS += $(shell geos-config --cflags)

# remove this if you do not want debugging to be compiled in
CXXFLAGS += -DOSMIUM_WITH_DEBUG

# Add this to force V8 garbage collection after each node/way/relation/area callback.
# Use only to find memory leaks. It will make osmjs really slow.
#CXXFLAGS += -DOSMIUM_V8_FORCE_GC

CXXFLAGS += -I../include

LDFLAGS = -L/usr/local/lib -lexpat
LDFLAGS += $(shell geos-config --libs)

LIB_V8    = -lv8 -licuuc
LIB_SHAPE = -lshp
LIB_PBF   = -lz -lpthread -lprotobuf-lite -losmpbf

.PHONY: all clean install

all: osmjs

osmjs: osmjs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS) $(LIB_PBF) $(LIB_V8) $(LIB_SHAPE)

install:
	install -m 755 -g root -o root -d $(DESTDIR)/usr/bin
	install -m 755 -g root -o root osmjs $(DESTDIR)/usr/bin/osmjs
	install -m 755 -g root -o root -d $(DESTDIR)/usr/share/osmjs
	install -m 644 -g root -o root js/osm2shape.js $(DESTDIR)/usr/share/osmjs/osm2shape.js
	install -m 755 -g root -o root -d $(DESTDIR)/usr/share/doc/osmjs
	install -m 644 -g root -o root README $(DESTDIR)/usr/share/doc/osmjs/README
    
clean:
	rm -f *.o core osmjs

deb:
	debuild -I -us -uc

deb-clean:
	debuild clean

