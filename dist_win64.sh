#!/bin/bash
DIR=holtz-win32;
i="cross64";
#i="cross";
VERSION=1.5.0

CROSS_COMPILE_DIR='/usr/x86_64-w64-mingw32/sys-root/mingw/bin'  #Attention: must be absolute path
# GCC_S_SJLJ='/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libgcc_s_seh-1.dll'
# STDCPP='/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libstdc++-6.dll' 

if ! test -d "$CROSS_COMPILE_DIR"; then
  echo "missing cross compiled library directory in: $CROSS_COMPILE_DIR (don't forget to cross compile wxWidgets)"
  exit -1;
fi
if ! test -f "$(ls "$CROSS_COMPILE_DIR"/libwx*.dll | head -n1)"; then
  echo "missing cross compiled wxWidgets libraries in: $CROSS_COMPILE_DIR/libwx*.dll"
  exit -2;
fi
# if ! test -f "$GCC_S_SJLJ"; then
#   echo "missing mingw GCC support library: $GCC_S_SJLJ"
#   exit -1;
# fi
# if ! test -f "$STDCPP"; then
#   echo "missing g++ standard library: $STDCPP"
#   exit -1;
# fi

#update EXE files
cp build-mingw64/holtz.exe exe/holtz-cross64.exe
strip exe/holtz-cross64.exe

FILES="";
echo Building zip-package for $i;
cp exe/holtz-${i}.exe $DIR/holtz.exe;
if cd holtz-win32-${i}; then 
  rm *.dll
  cp "$CROSS_COMPILE_DIR"/*.dll . 
  # cp "$GCC_S_SJLJ" libgcc_s_seh-1.dll
  # cp "$STDCPP" libstdc++-6.dll
  for j in *; do 
    if test -f $j; then 
      cp $j ../$DIR/$j; FILES="$FILES $DIR/$j"; 
    fi; 
  done; 
  cd -; 
fi;
zip -r holtz-${VERSION}-win32.zip $DIR/holtz.exe $DIR/AUTHORS.txt $DIR/COPYING.txt $DIR/skins/*.zip \
  $DIR/help/help_*.zip $DIR/sounds/*.wav $DIR/locale/*/LC_MESSAGES/*.mo $FILES;
if cd holtz-win32-${i}; then for j in *; do if test -f $j; then rm ../$DIR/$j; fi; done; cd -; fi;

