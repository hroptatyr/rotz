include $(top_builddir)/version.mk

# Help the Developers and yourself.  Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
LANG=C
LC_ALL=C

BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)
SUFFIXES =

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
built_mans += rotz-fsck.man
built_mans += rotz-grep.man

EXTRA_DIST += author.h2m

EXTRA_DIST += rotz-add.xmpl
EXTRA_DIST += rotz-del.xmpl
EXTRA_DIST += rotz-show.xmpl

VPATH += @top_srcdir@/src
SUFFIXES += .yuck
SUFFIXES += .man
.yuck.man:
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		yuck$(EXEEXT) genman \
			--package "$(PACKAGE_NAME)" \
			-i "$(srcdir)/author.h2m" \
			-o $@ --version-file "$(top_builddir)/.version" $<

## Makefile.am ends here
