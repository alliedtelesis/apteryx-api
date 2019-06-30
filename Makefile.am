lib_LTLIBRARIES = libapteryx-schema.la

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
