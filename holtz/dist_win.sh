#!/bin/bash
DIR=holtz-win32;
#i="cross32";
#VERSION=1.3.1
i="cross";
VERSION=1.4.0

#update EXE files
#cp build-mingw32/holtz.exe exe/holtz-cross32.exe

FILES="";
echo Building zip-package for $i;
cp exe/holtz-${i}.exe $DIR/holtz.exe;
if cd holtz-win32-${i}; then for j in *; do if test -f $j; then cp $j ../$DIR/$j; FILES="$FILES $DIR/$j"; fi; done; cd -; fi;
zip -r holtz-${VERSION}-win32.zip $DIR/holtz.exe $DIR/AUTHORS.txt $DIR/COPYING.txt $DIR/skins/*.zip \
  $DIR/help/help_*.zip $DIR/sounds/*.wav $DIR/locale/*/LC_MESSAGES/*.mo $FILES;
if cd holtz-win32-${i}; then for j in *; do if test -f $j; then rm ../$DIR/$j; fi; done; cd -; fi;

