lib_LTLIBRARIES = libapteryx-xml.la

libapteryx_xml_la_SOURCES = schema.c
if HAVE_LUA
libapteryx_xml_la_SOURCES += lua.c
endif

libapteryx_xml_la_CFLAGS = -D_GNU_SOURCE
if HAVE_LUA
libapteryx_xml_la_CFLAGS += @LUA_CFLAGS@
endif
libapteryx_xml_la_CFLAGS += @LIBXML_CFLAGS@ @APTERYX_CFLAGS@ @GLIB_CFLAGS@

libapteryx_xml_la_LIBADD =
if HAVE_LUA
libapteryx_xml_la_LIBADD += @LUA_LIBS@
endif
libapteryx_xml_la_LIBADD += @APTERYX_LIBS@ @GLIB_LIBS@

include_HEADERS = apteryx-xml.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = apteryx-xml.pc
