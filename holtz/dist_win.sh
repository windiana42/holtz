#!/bin/bash
DIR=holtz-win32;
BUILDS="msvc cygwin mingw"

for i in $BUILDS; do
  cp holtz-${i}.exe $DIR/;
  zip -r holtz-win32-${i}.zip $DIR/holtz.exe $DIR/AUTHORS $DIR/COPYING $DIR/skins/*.zip \
	$DIR/help/help_*.zip $DIR/sounds/*.wav $DIR/locale/*/LC_MESSAGES/*.mo;
done
