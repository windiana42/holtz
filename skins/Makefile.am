SUBDIRS = eins
skinsdir = $(pkgdatadir)/skins
skins_DATA = $(srcdir)/*.zip
EXTRA_DIST = $(srcdir)/*.zip $(srcdir)/skin.ini.defaults

pack:
	for i in `find -name skin -and -type d`; do \
	  skin_name=`echo $$i | sed "s/[^/]*\/\([^/]*\)\/skin/\1/"`; \
	  echo "Packing skin $(skin_name)..."; \
	  cd $i; \
	  zip ../../$(skin_name).zip *.png skin.ini; \
	  cd ../..; \
	done

