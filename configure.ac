AC_INIT([apteryx-api], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC_STDC
AM_PROG_AR

LT_INIT([disable-static pic-only])

AC_CONFIG_MACRO_DIRS([m4])

PKG_CHECK_MODULES([GLIB],[glib-2.0])
PKG_CHECK_MODULES([APTERYX],[apteryx])
PKG_CHECK_MODULES([LIBXML],[libxml-2.0])

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

AC_CONFIG_FILES([Makefile apteryx-xml.pc])
AC_OUTPUT
