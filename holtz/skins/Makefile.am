SUBDIRS = eins
skinsdir = $(pkgdatadir)/skins
skins_DATA = *.zip
EXTRA_DIST = *.zip skin.ini.defaults

pack:
	for i in `find -name skin -and -type d`; do \
	  skin_name=`echo $$i | sed "s/[^/]*\/\([^/]*\)\/skin/\1/"`; \
	  echo "Packing skin $(skin_name)..."; \
	  cd $i; \
	  zip ../../$(skin_name).zip *.png skin.ini; \
	  cd ../..; \
	done

