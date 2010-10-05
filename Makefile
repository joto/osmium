#------------------------------------------------------------------------------
#
#  Osmium Makefile
#
#------------------------------------------------------------------------------

CXX = g++

CXXFLAGS = -O3 -Wall -W -pedantic -Wredundant-decls -Wdisabled-optimization
#CXXFLAGS = -g -fPIC
#CXXFLAGS = -Wpadded -Winline

LDFLAGS = -lexpat -lpthread

LIB_V8     = -L/usr/local/lib libv8.a
LIB_SQLITE = -lsqlite3
LIB_GD     = -lgd -lpng -lz -lm

CPP = XMLParser.cpp HandlerStatistics.cpp wkb.cpp
CPP_JS = JavascriptTemplate.cpp

OBJ = XMLParser.o HandlerStatistics.o wkb.o
OBJ_JS = JavascriptTemplate.o

HPP = osmium.hpp Osm.hpp OsmObject.hpp OsmNode.hpp OsmWay.hpp OsmRelation.hpp XMLParser.hpp wkb.hpp StringStore.hpp Handler.hpp HandlerStatistics.hpp HandlerTagStats.hpp HandlerNodeLocationStore.hpp

SRC_CPP = $(patsubst %,src/%,$(CPP))
SRC_OBJ = $(patsubst %,src/%,$(OBJ))
SRC_HPP = $(patsubst %,src/%,$(HPP))
SRC_CPP_JS = $(patsubst %,src/%,$(CPP_JS))
SRC_OBJ_JS = $(patsubst %,src/%,$(OBJ_JS))
SRC_HPP_JS = $(patsubst %,src/%,$(HPP_JS))
SRC_HPP_JS = $(wildcard src/*Javascript*.hpp)

#sources = src/*.cpp
#include $(sources:.cpp=.d)

.PHONY: all clean doc

all: osmium_js osmium_tagstats

#src/%.d: src/%.cpp
#	$(SHELL) -ec "$(CXX) -MM -MT '$(patsubst %.cpp,%.o,$<) $(patsubst %.cpp,%.d,$<)' $< >$@; [ -s $@ ] || rm -f $@"

src/%.o: src/%.cpp $(SRC_HPP)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

osmium_js: src/osmium_js.cpp $(SRC_OBJ) $(SRC_OBJ_JS) $(SRC_HPP) $(SRC_HPP_JS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC_OBJ) $(SRC_OBJ_JS) $(LDFLAGS) $(LIB_V8)

osmium_tagstats: src/osmium_tagstats.cpp $(SRC_OBJ) $(SRC_HPP)
	$(CXX) $(CXXFLAGS) -o $@ $< $(SRC_OBJ) $(LDFLAGS) $(LIB_GD) $(LIB_SQLITE)
    
clean:
	rm -f src/*.o src/*.d osmium_js osmium_tagstats

doc: doc/html/files.html

doc/html/files.html: src/*.hpp src/*.cpp
	doxygen

