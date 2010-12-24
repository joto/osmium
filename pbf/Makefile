#------------------------------------------------------------------------------
#
#  Osmium pbf makefile
#
#------------------------------------------------------------------------------

# these will be overwritten from main Makefile if called from there
CXX = g++
CXXFLAGS = -g -std=c++0x -Wall -W -Wredundant-decls -Wdisabled-optimization -pedantic

all: fileformat.pb.o osmformat.pb.o fileformat.pb.h osmformat.pb.h

%.pb.o: %.pb.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.pb.cc %.pb.h: %.proto
	protoc --proto_path=. --cpp_out=. $<

clean:
	rm -f *.pb.h *.pb.cc *.pb.o

