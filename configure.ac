AC_INIT([libgdav],
        [0.1],
        [libgdav])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11.2 foreign -Wno-portability no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])

dnl Check for programs
AC_PROG_CC
LT_INIT
PKG_PROG_PKG_CONFIG
IT_PROG_INTLTOOL([0.40.0])

AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])

dnl Library Dependencies

dnl Keep these two definitions in agreement.
m4_define([glib_minimum_version], [2.36])
m4_define([glib_encoded_version], [GLIB_VERSION_2_36])

dnl Keep these two definitions in agreement.
m4_define([soup_minimum_version], [2.42])
m4_define([soup_encoded_version], [SOUP_VERSION_2_42])

PKG_CHECK_MODULES(GIO, gio-2.0 >= glib_minimum_version)
PKG_CHECK_MODULES(LIBSOUP, libsoup-2.4 >= soup_minimum_version)
PKG_CHECK_MODULES(LIBXML2, libxml-2.0)

dnl Extra Tools Dependencies
PKG_CHECK_MODULES(LIBEDIT, libedit)

dnl Documentation
GTK_DOC_CHECK([1.14])

dnl Internationalization
GETTEXT_PACKAGE=libgdav
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"],
                   [Define the gettext package to be used])

dnl Compiler Warnings
AS_COMPILER_FLAGS(WARNING_FLAGS,
	"-Wall \
	-Wstrict-prototypes \
	-Werror=missing-prototypes \
	-Werror=implicit-function-declaration \
	-Werror=pointer-arith \
	-Werror=init-self \
	-Werror=format=2 \
	-Wno-format-zero-length \
	-Werror=missing-include-dirs \
	-Werror=aggregate-return \
	-Werror=declaration-after-statement \
	-Werror=redundant-decls")
AC_SUBST(WARNING_FLAGS)

AM_CFLAGS="$WARNING_FLAGS"
AC_SUBST(AM_CFLAGS)

AM_CPPFLAGS=

dnl Warn about API usage that violates our minimum requirements.
AM_CPPFLAGS="$AM_CPPFLAGS -DGLIB_VERSION_MAX_ALLOWED=glib_encoded_version"
AM_CPPFLAGS="$AM_CPPFLAGS -DSOUP_VERSION_MAX_ALLOWED=soup_encoded_version"

dnl These will suppress warnings about newly-deprecated symbols.
AM_CPPFLAGS="$AM_CPPFLAGS -DGLIB_VERSION_MIN_REQUIRED=glib_encoded_version"
AM_CPPFLAGS="$AM_CPPFLAGS -DSOUP_VERSION_MIN_REQUIRED=soup_encoded_version"

AC_SUBST(AM_CPPFLAGS)

AC_CONFIG_FILES([
  docs/Makefile
  libgdav/Makefile
  po/Makefile.in
  tools/Makefile
  Makefile
])
AC_OUTPUT
