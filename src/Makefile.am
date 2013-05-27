### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=201001L -D_XOPEN_SOURCE=800 -D_BSD_SOURCE
AM_LDFLAGS = $(XCCLDFLAGS)

bin_PROGRAMS =
noinst_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
pkglib_LTLIBRARIES =
noinst_HEADERS =
BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)
CLEANFILES = 

CLEANFILES += version.c
EXTRA_DIST += version.c.in
noinst_HEADERS += version.h

lib_LTLIBRARIES += librotz.la
librotz_la_SOURCES = rotz.c rotz.h
librotz_la_SOURCES += nifty.h
librotz_la_CPPFLAGS = $(AM_CPPFLAGS)
librotz_la_CPPFLAGS += $(tokyocabinet_CFLAGS)
librotz_la_LDFLAGS = $(AM_LDFLAGS)
librotz_la_LDFLAGS += $(tokyocabinet_LIBS)
nodist_librotz_la_SOURCES = version.c

bin_PROGRAMS += rotz-add
rotz_add_SOURCES = rotz-add.c
rotz_add_SOURCES += rotz-cmd-api.h
rotz_add_SOURCES += rotz-add-clo.h rotz-add-clo.ggo
rotz_add_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_add_CPPFLAGS += $(tokyocabinet_CFLAGS)
rotz_add_LDFLAGS = $(AM_LDFLAGS)
rotz_add_LDFLAGS += $(tokyocabinet_LIBS)
rotz_add_LDADD = librotz.la
BUILT_SOURCES += rotz-add-clo.c rotz-add-clo.h

bin_PROGRAMS += rotz-del
rotz_del_SOURCES = rotz-del.c
rotz_del_SOURCES += rotz-cmd-api.h
rotz_del_SOURCES += rotz-del-clo.h rotz-del-clo.ggo
rotz_del_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_del_CPPFLAGS += $(tokyocabinet_CFLAGS)
rotz_del_LDFLAGS = $(AM_LDFLAGS)
rotz_del_LDFLAGS += $(tokyocabinet_LIBS)
rotz_del_LDADD = librotz.la
BUILT_SOURCES += rotz-del-clo.c rotz-del-clo.h

bin_PROGRAMS += rotz-show
rotz_show_SOURCES = rotz-show.c
rotz_show_SOURCES += rotz-cmd-api.h
rotz_show_SOURCES += rotz-show-clo.h rotz-show-clo.ggo
rotz_show_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_show_CPPFLAGS += $(tokyocabinet_CFLAGS)
rotz_show_LDFLAGS = $(AM_LDFLAGS)
rotz_show_LDFLAGS += $(tokyocabinet_LIBS)
rotz_show_LDADD = librotz.la
BUILT_SOURCES += rotz-show-clo.c rotz-show-clo.h

version.c: version.c.in $(top_builddir)/version.mk
	sed -e 's,[@]VERSION[@],$(VERSION),g' <$(srcdir)/$@.in >$@

## gperf rule
%.c: %.gperf
	$(AM_V_GEN) $(GPERF) -L ANSI-C $< --output-file $@

## gengetopt rule with sed hack to avoid using VERSION and config.h
## depend on Makefile.am to survive a git pull
%.c %.h: %.ggo Makefile.am
	$(AM_V_GEN) gengetopt -l -i $< -F $* \
	&& sed \
	  -e 's,config.h,version.h,g' \
	  -e 's,\(.*\)_PARSER_VERSION VERSION,\1_PARSER_VERSION $(PACKAGE)_version_string,g' \
	  $*.h >$*.h-t \
	&& mv $*.h-t $*.h

## for dist or dist check we need binaries, so help2man can work
dist-hook: $(bin_PROGRAMS)


## Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<
