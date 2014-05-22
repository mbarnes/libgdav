AC_INIT([libgdav],
        [0.1],
        [libgdav])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11.2 foreign -Wno-portability no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])

# Check for programs
AC_PROG_CC
LT_INIT
PKG_PROG_PKG_CONFIG
IT_PROG_INTLTOOL([0.40.0])

AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])

# Library Dependencies
PKG_CHECK_MODULES(GIO, gio-2.0 >= 2.36)  # Just a guess
PKG_CHECK_MODULES(LIBSOUP, libsoup-2.4 >= 2.42)  # Just a guess
PKG_CHECK_MODULES(LIBXML2, libxml-2.0)

# Extra Tools Dependencies
PKG_CHECK_MODULES(LIBEDIT, libedit)

# Documentation
GTK_DOC_CHECK([1.14])

# Internationalization
GETTEXT_PACKAGE=libgdav
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"],
                   [Define the gettext package to be used])

AC_CONFIG_FILES([
  docs/Makefile
  libgdav/Makefile
  po/Makefile.in
  tools/Makefile
  Makefile
])
AC_OUTPUT
