include $(top_builddir)/version.mk

# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)

man1_MANS =
man1_MANS += $(built_mans)

EXTRA_DIST += author.h2m
BUILT_SOURCES += $(built_mans) $(built_infos)

built_mans =
built_mans += rotz-add.man
built_mans += rotz-del.man
built_mans += rotz-show.man
built_mans += rotz-alias.man
built_mans += rotz-combine.man
built_mans += rotz-search.man
built_mans += rotz-rename.man
built_mans += rotz-cloud.man

EXTRA_DIST += rotz-add.name
EXTRA_DIST += rotz-del.name
EXTRA_DIST += rotz-show.name
EXTRA_DIST += rotz-alias.name
EXTRA_DIST += rotz-combine.name
EXTRA_DIST += rotz-search.name
EXTRA_DIST += rotz-rename.name
EXTRA_DIST += rotz-cloud.name

EXTRA_DIST += rotz-add.xmpl
EXTRA_DIST += rotz-del.xmpl
EXTRA_DIST += rotz-show.xmpl

VPATH += $(top_srcdir)/src

## help2man helpers
%.man: %-clo.ggo $(top_srcdir)/configure $(top_builddir)/version.mk
	$(AM_V_GEN) $(HELP2MAN) -S "$(PACKAGE_STRING)" -p "($(PACKAGE))$*" \
		-I "$(srcdir)/author.h2m" \
		-i "$(srcdir)/$*.name" \
		-I "$(srcdir)/$*.xmpl" \
		-o $@ $(top_builddir)/src/$*

