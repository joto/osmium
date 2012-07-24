#------------------------------------------------------------------------------
#
#  Osmium main makefile
#
#------------------------------------------------------------------------------

all:

clean:
	rm -fr check-includes-report doc/html test/tests

install: doc
	install -m 755 -g root -o root -d $(DESTDIR)/usr/include
	install -m 755 -g root -o root -d $(DESTDIR)/usr/share/doc/libosmium-dev
	install -m 644 -g root -o root README $(DESTDIR)/usr/share/doc/libosmium-dev/README
	install -m 644 -g root -o root include/osmium.hpp $(DESTDIR)/usr/include
	cp --recursive include/osmium $(DESTDIR)/usr/include
	cp --recursive doc/html $(DESTDIR)/usr/share/doc/libosmium-dev

check:
	cppcheck --enable=all -I include */*.cpp test/*/test_*.cpp

# This will try to compile each include file on its own to detect missing
# #include directives. Note that if this reports [OK], it is not enough
# to be sure it will compile in production code. But if it reports [FAILED]
# we know we are missing something.
check-includes:
	echo "CHECK INCLUDES REPORT:" >check-includes-report; \
	for FILE in include/*.hpp include/*/*.hpp include/*/*/*.hpp include/*/*/*/*.hpp; do \
        flags=`./get_options.sh --cflags $${FILE}`; \
        eval eflags=$${flags}; \
        compile="g++ -I include $${eflags} $${FILE}"; \
        echo "\n======== $${FILE}:\n$${compile}" >>check-includes-report; \
        echo -n "$${FILE} "; \
        if `$${compile} 2>>check-includes-report`; then \
            echo "[OK]"; \
        else \
            echo "[FAILED]"; \
        fi; \
        rm -f $${FILE}.gch; \
	done

indent:
	astyle --style=java --indent-namespaces --indent-switches --pad-header --suffix=none --recursive include/\*.hpp examples/\*.cpp examples/\*.hpp osmjs/\*.cpp test/\*.cpp

doc: doc/html/files.html Doxyfile

doc/html/files.html: include/*.hpp include/*/*.hpp include/*/*/*.hpp
	doxygen >/dev/null

deb:
	debuild -I -us -uc

deb-clean:
	debuild clean

