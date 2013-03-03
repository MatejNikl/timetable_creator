ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS = timetable_creator
timetable_creator_SOURCES = src/main.c src/structs.c src/timetable.c \
                            src/structs.h src/timetable.h src/gettext.h
timetable_creator_LDADD = $(LIBINTL) $(LIBOBJS) $(GLIB_LIBS) $(GTK3_LIBS)
timetable_creator_CPPFLAGS = -DGETTEXT_PACKAGE=\"timetable_creator\" \
                             $(GLIB_CFLAGS) $(GTK3_CFLAGS) $(AM_CPPFLAGS)

dist_doc_DATA = COPYING NEWS README AUTHORS
EXTRA_DIST = build-aux/config.rpath

SUBDIRS = po
