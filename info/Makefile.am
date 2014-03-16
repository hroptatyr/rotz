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
SUFFIXES =

built_mans =
built_mans += rotz.man

EXTRA_DIST += rotz-add.xmpl
EXTRA_DIST += rotz-del.xmpl
EXTRA_DIST += rotz-show.xmpl

VPATH += @top_srcdir@/src

## help2man helpers
SUFFIXES += .yuck
SUFFIXES += .man
.yuck.man:
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		yuck$(EXEEXT) genman \
			--package "$(PACKAGE_NAME)" \
			-i "$(srcdir)/rotz-add.xmpl" \
			-i "$(srcdir)/rotz-del.xmpl" \
			-i "$(srcdir)/rotz-show.xmpl" \
			-i "$(srcdir)/author.h2m" \
			-o $@ --version-string "$(VERSION)" $<

