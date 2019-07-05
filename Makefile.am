lib_LTLIBRARIES = libapteryx_schema.la

libapteryx_schema_la_SOURCES = schema.c
if HAVE_LIBXML
libapteryx_schema_la_SOURCES += xml.c
endif
if HAVE_LIBYANG
libapteryx_schema_la_SOURCES += yang.c
endif
if HAVE_LUA
libapteryx_schema_la_SOURCES += lua.c
endif

libapteryx_schema_la_CFLAGS = -D_GNU_SOURCE
if HAVE_LIBXML
libapteryx_schema_la_CFLAGS += -DHAVE_LIBXML @LIBXML_CFLAGS@
endif
if HAVE_LIBYANG
libapteryx_schema_la_CFLAGS += -DHAVE_LIBYANG @LIBYANG_CFLAGS@
endif
if HAVE_LUA
libapteryx_schema_la_CFLAGS += -DHAVE_LUA @LUA_CFLAGS@
endif
libapteryx_schema_la_CFLAGS += @APTERYX_CFLAGS@ @GLIB_CFLAGS@

libapteryx_schema_la_LDFLAGS = -version-info 1:0:0

libapteryx_schema_la_LIBADD =
if HAVE_LIBXML
libapteryx_schema_la_LIBADD += @LIBXML_LIBS@
endif
if HAVE_LIBYANG
libapteryx_schema_la_LIBADD += @LIBYANG_LIBS@
endif
if HAVE_LUA
libapteryx_schema_la_LIBADD += @LUA_LIBS@
endif
libapteryx_schema_la_LIBADD += @APTERYX_LIBS@ @GLIB_LIBS@

include_HEADERS = apteryx-schema.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = apteryx-schema.pc

if HAVE_TESTS
noinst_PROGRAMS = unittest
unittest_SOURCES = test.c $(libapteryx_schema_la_SOURCES)
unittest_CFLAGS = $(libapteryx_schema_la_CFLAGS) -g -fprofile-arcs -fprofile-dir=gcov -ftest-coverage
unittest_LDADD = $(libapteryx_schema_la_LIBADD)

# TEST_WRAPPER="LD_PRELOAD=.libs/libapteryx_schema.so G_SLICE=always-malloc valgrind --leak-check=full" make test
# TEST_WRAPPER="gdb" make test
# make test TEST_ARGS="-h"
test: libapteryx_schema.la unittest
	@echo "Running unit tests"
	@if test -e /tmp/apteryxd.pid; then \
		kill -TERM `cat /tmp/apteryxd.pid` && sleep 0.1; \
	fi; \
	rm -f /tmp/apteryxd.pid; \
	rm -f /tmp/apteryxd.run; \
	apteryxd -b -p /tmp/apteryxd.pid -r /tmp/apteryxd.run && sleep 0.1; \
	mkdir -p gcov; \
	eval LD_LIBRARY_PATH=/usr/local/lib LUA_CPATH=.libs/?.so $(TEST_WRAPPER) ./unittest $(TEST_ARGS); \
	mv *.gcno gcov/ 2> /dev/null; \
	lcov -q --capture --directory . --output-file gcov/coverage.info; \
	genhtml -q gcov/coverage.info --output-directory gcov; \
	kill -TERM `cat /tmp/apteryxd.pid`;
	@echo "Tests have been run!"
endif