#------------------------------------------------------------------------------
#
#  Osmium Protobuf Makefile
#
#------------------------------------------------------------------------------

CXX = g++

CXXFLAGS = -g
#CXXFLAGS = -O3

CXXFLAGS += -std=c++0x -Wall -W -Wredundant-decls -Wdisabled-optimization -pedantic

FILES = fileformat.pb.o osmformat.pb.o fileformat.pb.cc osmformat.pb.cc fileformat.pb.h osmformat.pb.h
all: $(FILES)

%.pb.o: %.pb.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.pb.cc %.pb.h: %.proto
	protoc --proto_path=. --cpp_out=. $<

clean:
	rm -f $(FILES)

