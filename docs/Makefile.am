include gtk-doc.make

NULL =

DOC_MODULE = libgdav

DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.sgml

DOC_SOURCE_DIR = $(top_srcdir)/libgdav

HFILE_GLOB = $(top_srcdir)/libgdav/*.h
CFILE_GLOB = $(top_srcdir)/libgdav/*.c

MKDB_OPTIONS = --output-format=xml --sgml-mode

GTKDOC_CFLAGS = \
	-I$(top_srcdir) \
	$(LIBSOUP_CFLAGS) \
	$(LIBXML2_CFLAGS) \
	$(GIO_CFLAGS) \
	$(NULL)

GTKDOC_LIBS = \
	$(top_builddir)/libgdav/libgdav.la \
	$(LIBSOUP_LIBS) \
	$(LIBXML2_LIBS) \
	$(GIO_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
