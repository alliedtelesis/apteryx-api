AC_INIT([apteryx_schema], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC_STDC
AM_PROG_AR

LT_INIT([disable-static pic-only])

AC_CONFIG_MACRO_DIRS([m4])

PKG_CHECK_MODULES([GLIB],[glib-2.0])
PKG_CHECK_MODULES([APTERYX],[apteryx])

AC_ARG_ENABLE([xml],
[  --enable-xml            support XML based schema (default=yes)],
[case "${enableval}" in
  yes) xml=true ;;
  no)  xml=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-xml]) ;;
esac],[xml=true])
AM_CONDITIONAL([HAVE_LIBXML], [test x$xml = xtrue])
if test x$xml = xtrue; then
    PKG_CHECK_MODULES([LIBXML],[libxml-2.0])
fi

AC_ARG_ENABLE([yang],
[  --enable-yang           support Yang based schema (default=yes)],
[case "${enableval}" in
  yes) yang=true ;;
  no)  yang=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-yang]) ;;
esac],[yang=true])
AM_CONDITIONAL([HAVE_LIBYANG], [test x$yang = xtrue])
if test x$yang = xtrue; then
    PKG_CHECK_MODULES([LIBYANG],[libyang])
fi

AC_ARG_ENABLE([lua],
[  --enable-lua            build lua api (default=yes)],
[case "${enableval}" in
  yes) lua=true ;;
  no)  lua=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-lua]) ;;
esac],[lua=true])
AM_CONDITIONAL([HAVE_LUA], [test x$lua = xtrue])
if test x$lua = xtrue; then
    PKG_CHECK_MODULES(LUA, [lua5.3],,[PKG_CHECK_MODULES(LUA, [lua5.2])])
fi

AC_ARG_ENABLE([tests],
[  --enable-tests          build unit tests (default=yes)],
[case "${enableval}" in
  yes) tests=true ;;
  no)  tests=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-tests]) ;;
esac],[tests=true])
AM_CONDITIONAL([HAVE_TESTS], [test x$tests = xtrue])

AC_CONFIG_FILES([Makefile apteryx-schema.pc])
AC_OUTPUT
