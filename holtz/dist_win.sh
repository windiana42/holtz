#!/bin/bash
DIR=holtz-win32;
BUILDS="msvc cygwin mingw"

for i in $BUILDS; do
  FILES="";
  echo Building zip-package for $i;
  cp holtz-${i}.exe $DIR/holtz.exe;
  if cd holtz-win32-${i}; then for j in *; do if test -f $j; then cp $j ../$DIR/$j; FILES="$FILES $DIR/$j"; fi; done; cd -; fi;
  zip -r holtz-win32-${i}.zip $DIR/holtz.exe $DIR/AUTHORS $DIR/COPYING $DIR/skins/*.zip \
	$DIR/help/help_*.zip $DIR/sounds/*.wav $DIR/locale/*/LC_MESSAGES/*.mo $FILES;
  if cd holtz-win32-${i}; then for j in *; do if test -f $j; then rm ../$DIR/$j; fi; done; cd -; fi;
done
