NULL =

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libgdav tools docs po

EXTRA_DIST = \
	m4 \
	enumtypes.c.template \
	enumtypes.h.template \
	$(NULL)

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

MAINTAINERCLEANFILES = \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/config.h.in \
	$(srcdir)/config.guess \
	$(srcdir)/config.sub \
	$(srcdir)/depcomp \
	$(srcdir)/install-sh \
	$(srcdir)/ltmain.sh \
	$(srcdir)/missing \
	$(srcdir)/docs/gtk-doc.make \
	$(srcdir)/m4/gtk-doc.m4 \
	$(srcdir)/m4/intltool.m4 \
	$(srcdir)/m4/libtool.m4 \
	$(srcdir)/m4/lt~obsolete.m4 \
	$(srcdir)/m4/ltoptions.m4 \
	$(srcdir)/m4/ltsugar.m4 \
	$(srcdir)/m4/ltversion.m4 \
	$(NULL)

-include $(top_srcdir)/git.mk
