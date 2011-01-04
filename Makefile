#------------------------------------------------------------------------------
#
#  Osmium main makefile
#
#------------------------------------------------------------------------------

all:
	$(MAKE) -C tagstats
	$(MAKE) -C osmjs

clean:
	$(MAKE) -C pbf clean
	$(MAKE) -C tagstats clean
	$(MAKE) -C osmjs clean

install:
	$(MAKE) -C tagstats install
	$(MAKE) -C osmjs install

doc: doc/html/files.html

doc/html/files.html: include/*.hpp src/*.cpp pbf/*.proto
	doxygen

