#------------------------------------------------------------------------------
#
#  Osmium main makefile
#
#------------------------------------------------------------------------------

all:
	$(MAKE) -C osmjs
	$(MAKE) -C examples

clean:
	$(MAKE) -C osmjs clean
	$(MAKE) -C examples clean

install:
	$(MAKE) -C osmjs install
	$(MAKE) -C examples install

check:
	cppcheck --enable=all -I include */*.cpp

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --suffix=none --recursive include/\*.hpp examples/\*.cpp examples/\*.hpp osmjs/\*.cpp

doc: doc/html/files.html

doc/html/files.html: include/*.hpp
	doxygen

deb:
	debuild -I -us -uc

deb-clean:
	debuild clean

