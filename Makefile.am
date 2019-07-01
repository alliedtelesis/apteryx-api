lib_LTLIBRARIES = libapteryx_schema.la

libapteryx_schema_la_SOURCES = schema.c
if HAVE_LUA
libapteryx_schema_la_SOURCES += lua.c
endif

libapteryx_schema_la_CFLAGS = -D_GNU_SOURCE
if HAVE_LIBXML
libapteryx_schema_la_CFLAGS += @LIBXML_CFLAGS@
endif
if HAVE_LUA
libapteryx_schema_la_CFLAGS += @LUA_CFLAGS@
endif
libapteryx_schema_la_CFLAGS += @APTERYX_CFLAGS@ @GLIB_CFLAGS@

libapteryx_schema_la_LDFLAGS = -version-info 1:0:0

libapteryx_schema_la_LIBADD =
if HAVE_LIBXML
libapteryx_schema_la_LIBADD += @LIBXML_LIBS@
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
unittest_SOURCES = test.c
unittest_CFLAGS = $(libapteryx_schema_la_CFLAGS)
unittest_LDADD = -lcunit -lapteryx_schema $(libapteryx_schema_la_LIBADD)

test: libapteryx_schema.la unittest
	@echo "Running unit tests"
	./test.sh
	@echo "Tests have been run!"
endif
