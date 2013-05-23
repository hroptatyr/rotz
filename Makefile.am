include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

SUBDIRS =
SUBDIRS += src

EXTRA_DIST =
DISTCLEANFILES =

EXTRA_DIST += git-version-gen
EXTRA_DIST += m4/.dir

DISTCLEANFILES += version.mk

dist-hook:
	echo $(VERSION) > $(distdir)/version \
	&& touch -r $(top_builddir)/version.mk $(distdir)/version
