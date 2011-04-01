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

check:
	cppcheck --enable=all -I include */*.cpp

doc: doc/html/files.html

doc/html/files.html: include/*.hpp src/*.cpp pbf/*.proto
	doxygen

deb:
	debuild -I -us -uc

deb-clean:
	debuild clean

