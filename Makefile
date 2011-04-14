#------------------------------------------------------------------------------
#
#  Osmium main makefile
#
#------------------------------------------------------------------------------

all:
	$(MAKE) -C src
	$(MAKE) -C osmjs
	$(MAKE) -C nodedensity

clean:
	$(MAKE) -C src clean
	$(MAKE) -C osmjs clean
	$(MAKE) -C nodedensity clean

install:
	$(MAKE) -C src install
	$(MAKE) -C osmjs install
	$(MAKE) -C nodedensity install

check:
	cppcheck --enable=all -I include */*.cpp

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --suffix=none --recursive include/\*.hpp src/\*.cpp nodedensity/\*.cpp nodedensity/\*.hpp tagstats/\*.hpp tagstats/\*.cpp osmjs/\*.cpp

doc: doc/html/files.html

doc/html/files.html: include/*.hpp
	doxygen

deb:
	debuild -I -us -uc

deb-clean:
	debuild clean

