NULL =

ENUM_TYPES = enums.h
ENUM_GENERATED = enumtypes.h enumtypes.c

enumtypes.h: $(top_srcdir)/enumtypes.h.template $(ENUM_TYPES)
	$(AM_V_GEN) $(GLIB_MKENUMS) \
	--identifier-prefix "GDav" \
	--template $(top_srcdir)/enumtypes.h.template \
	--fhead "#ifndef __ENUMTYPES_H__\n#define __ENUMTYPES_H__\n" \
	--ftail "#endif /* __ENUMTYPES_H__ */\n" \
	$(addprefix $(srcdir)/,$(ENUM_TYPES)) > $@

enumtypes.c: $(top_srcdir)/enumtypes.c.template $(ENUM_TYPES)
	$(AM_V_GEN) $(GLIB_MKENUMS) \
	--identifier-prefix "GDav" \
	--template $(top_srcdir)/enumtypes.c.template \
	--fhead "#include \"enumtypes.h\"" \
	$(addprefix $(srcdir)/,$(ENUM_TYPES)) > $@

BUILT_SOURCES = $(ENUM_GENERATED)

bin_PROGRAMS = gdav

gdav_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"gdav\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	$(NULL)

gdav_CFLAGS = \
	$(LIBEDIT_CFLAGS) \
	$(LIBSOUP_CFLAGS) \
	$(LIBXML2_CFLAGS) \
	$(GIO_CFLAGS) \
	$(NULL)

gdav_SOURCES = \
	$(BUILT_SOURCES) \
	main.c \
	commands.c \
	commands.h \
	enums.h \
	structs.h \
	utils.c \
	utils.h \
	$(NULL)

gdav_LDADD = \
	$(top_builddir)/libgdav/libgdav.la \
	$(LIBEDIT_LIBS) \
	$(LIBSOUP_LIBS) \
	$(LIBXML2_LIBS) \
	$(GIO_LIBS) \
	$(NULL)

-include $(top_srcdir)/git.mk
