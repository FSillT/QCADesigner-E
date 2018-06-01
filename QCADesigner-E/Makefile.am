## Process this file with automake to produce Makefile.in

docdir = $(datadir)/doc/@PACKAGE@-@VERSION@
appdir = $(datadir)/applications
qcasampledir = $(datadir)/@PACKAGE@/qca_cell_library

SUBDIRS = src po docs pixmaps

doc_DATA = \
  AUTHORS \
  COPYING \
  ChangeLog \
  README \
  README.win32.txt

qcasample_DATA = \
  sample.qca

app_DATA = QCADesigner.desktop graph_dialog.desktop

EXTRA_DIST = $(doc_DATA) $(app_DATA)

squeaky : distclean
	rm -rf \
	Output \
	Makefile Makefile.in aclocal.m4 autom4te.cache \
	compile config.log config.status configure depcomp \
	install-sh missing mkinstalldirs \
	config.guess config.sub \
	src/Makefile.in \
	docs/Makefile.in \
	pixmaps/Makefile.in \
	QCADesigner.desktop \
	graph_dialog.desktop \
	QCADesigner-win32.iss \
	po/Makefile po/Makefile.in po/Makefile.in.in \
	po/POTFILES po/*.gmo `find . | grep '.#'` *~
