#------------------------------------------------------------------------------
#
#  Osmium osmjs Makefile
#
#------------------------------------------------------------------------------
#
#  You can set several environment variables before running make if you don't
#  like the defaults:
#
#  CXX                - Your C++ compiler.
#  CPLUS_INCLUDE_PATH - Include file search path.
#  CXXFLAGS           - Extra compiler flags.
#  LDFLAGS            - Extra linker flags.
#  
#------------------------------------------------------------------------------

CXXFLAGS += -O3
#CXXFLAGS += -g
CXXFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CXXFLAGS += -I../include -I/usr/include/libshp

# remove this if you do not want debugging to be compiled in
CXXFLAGS += -DOSMIUM_WITH_DEBUG

# Add this to force V8 garbage collection after each node/way/relation/area callback.
# Use only to find memory leaks. It will make osmjs really slow.
#CXXFLAGS += -DOSMIUM_V8_FORCE_GC

CXXFLAGS_GEOS     := $(shell geos-config --cflags)
CXXFLAGS_WARNINGS := -Wall -Wextra -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long

LIB_EXPAT := -lexpat
LIB_PBF   := -lz -lpthread -lprotobuf-lite -losmpbf
LIB_V8    := -lv8 -licuuc
LIB_SHAPE := -lshp
LIB_GEOS  := $(shell geos-config --libs)

.PHONY: all install clean deb deb-clean

all: osmjs

osmjs: osmjs.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_GEOS) $(CXXFLAGS_WARNINGS) -o $@ $< $(LDFLAGS) $(LIB_EXPAT) $(LIB_PBF) $(LIB_V8) $(LIB_SHAPE) $(LIB_GEOS)

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

