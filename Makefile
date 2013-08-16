#------------------------------------------------------------------------------
#
#  Osmium main makefile
#
#------------------------------------------------------------------------------

CXX=g++

all:

.PHONY: clean install check check-includes test indent

clean:
	rm -fr check-includes-report doc/html
	$(MAKE) -C test clean

install: doc
	install -m 755 -g root -o root -d $(DESTDIR)/usr/include
	install -m 755 -g root -o root -d $(DESTDIR)/usr/share/doc/libosmium-dev
	install -m 644 -g root -o root README $(DESTDIR)/usr/share/doc/libosmium-dev/README
	install -m 644 -g root -o root include/osmium.hpp $(DESTDIR)/usr/include
	cp --recursive include/osmium $(DESTDIR)/usr/include
	cp --recursive doc/html $(DESTDIR)/usr/share/doc/libosmium-dev

check:
	cppcheck --enable=all -I include */*.cpp test/t/*/test_*.cpp

WARNINGFLAGS = -Wall -Wextra -Wredundant-decls -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long -Winline -Weffc++ -Wold-style-cast

# This will try to compile each include file on its own to detect missing
# #include directives. Note that if this reports [OK], it is not enough
# to be sure it will compile in production code. But if it reports an error
# we know we are missing something.
check-includes:
	echo "CHECK INCLUDES REPORT:" >check-includes-report; \
    allok=yes; \
	for FILE in include/*.hpp include/*/*.hpp include/*/*/*.hpp include/*/*/*/*.hpp; do \
        flags=`./get_options.sh --cflags $${FILE}`; \
        eval eflags=$${flags}; \
        compile="$(CXX) $(WARNINGFLAGS) -I include $${eflags} $${FILE}"; \
        echo "\n======== $${FILE}\n$${compile}" >>check-includes-report; \
        if `$${compile} 2>>check-includes-report`; then \
            echo "[OK] $${FILE}"; \
        else \
            echo "[  ] $${FILE}"; \
            allok=no; \
        fi; \
        rm -f $${FILE}.gch; \
	done; \
    if test $${allok} = "yes"; then echo "All files OK"; else echo "There were errors"; fi; \
    echo "\nDONE" >>check-includes-report

test:
	(cd test && ./run_tests.sh)

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --lineend=linux --suffix=none --recursive include/\*.hpp examples/\*.cpp examples/\*.hpp osmjs/\*.cpp test/\*.cpp
#	astyle --style=java --indent-namespaces --indent-switches --pad-header --unpad-paren --align-pointer=type --lineend=linux --suffix=none --recursive include/\*.hpp examples/\*.cpp examples/\*.hpp osmjs/\*.cpp test/\*.cpp

doc: doc/html/files.html Doxyfile

doc/html/files.html: include/*.hpp include/*/*.hpp include/*/*/*.hpp
	doxygen >/dev/null

deb:
	debuild -I -us -uc

deb-clean:
	debuild clean

