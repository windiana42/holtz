AC_INIT(wxmain.cpp)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE(holtz, 1.5.0)

WXLIBS=
#I thought this would be right, but it actually doesn't work:
#WXLIBS=adv # this adds additional libs (ex: for sound in the advanced package of wxWidgets)

echo "target=$target";
echo "target_os=$target_os";
echo "target_vendor=$target_vendor";

case $target_os in
*cygwin* )
	echo "Target System = Cygwin:";
	WXLIBS=; # adv doesn't seem to do good for cygwin version
        AC_CHECK_TOOL(WINDRES, windres) 
	WINDRES_FLAGS="$wxconfig_EXEC $GTKVER $STATIC $UNICODE --cflags"
	AC_SUBST(WINDRES_FLAGS)
	AM_CONDITIONAL([INCLUDE_RC_FILE],[test 1 -eq 1]);;
#	export CXXFLAGS="$CXXFLAGS -mno-cygwin -D__CYGWIN__"; # set true
#	export LIBS="$LIBS -lwinmm";
#	AC_CHECK_LIB(winmm, PlaySound);;
*mingw*  ) 
	echo "Target System = MinGW"; 
        AC_CHECK_TOOL(WINDRES, windres) 
	AM_CONDITIONAL([INCLUDE_RC_FILE],[test 1 -eq 1]);;    # set true
#	export LIBS="$LIBS -lwinmm";
#	AC_CHECK_LIB(winmm, PlaySound);;	   
* ) 
	echo "Target System is not Windows target: $target_os"; 
	AM_CONDITIONAL([INCLUDE_RC_FILE],[test 0 -eq 1]);;    # set false
#	AC_CHECK_HEADER( unistd.h, export CXXFLAGS="$CXXFLAGS -D__UNIX__" );;
esac

wx_2_6_Enabled="no"
wxstatic_Enabled="no"
unicode_Enabled="yes"
wxconfig_EXEC="wx-config"

AC_ARG_ENABLE(wxstatic,AC_HELP_STRING([--enable-wxstatic=VAL],
	[VAL=yes: Link WX libraries static; 
	 VAL=no: Link WX as shared library]),
	[wxstatic_Enabled="$enableval"])

AC_ARG_ENABLE(unicode,AC_HELP_STRING([--enable-unicode=VAL],
	[VAL=yes: Link to unicode version of wxWidgets (wx-config --unicode); 
	 VAL=no: use default wxGTK Version]),
	[unicode_Enabled="$enableval"])

AC_ARG_ENABLE(wx_2_6,AC_HELP_STRING([--enable-wx_2_6=VAL],
	[VAL=yes: Make code wxWidgets 2.6 compatible; 
	 VAL=no: use default wxGTK Version]),
	[wx_2_6_Enabled="$enableval"])

AC_ARG_ENABLE(wxconfig,AC_HELP_STRING([--enable-wxconfig=VAL],
	[VAL=wx-config: PATH of wx-config executable]),
	[wxconfig_EXEC="$enableval"])

GTKVER=
STATIC=
UNICODE=
BACKCOMPATIBILITY=

if test "$wxstatic_Enabled" = "yes" ; then
  STATIC="--static=yes"
else
  STATIC="--static=no"
fi

if test "$unicode_Enabled" = "yes" ; then
  UNICODE="--unicode=yes"
else
  UNICODE="--unicode=no"
fi

if test "$wx_2_6_Enabled" = "yes" ; then
  BACKCOMPATIBILITY="-DBACKCOMPATIBLE_WX_2_6"
  echo "Activating backward compatibility for wxWidgets 2.6";
fi

echo "$wxconfig_EXEC $GTKVER $STATIC $UNICODE --libs $WXLIBS";
export LIBS="$LIBS `$wxconfig_EXEC $GTKVER $STATIC $UNICODE --libs $WXLIBS`";
export CXXFLAGS="$CXXFLAGS $BACKCOMPATIBILITY `$wxconfig_EXEC $GTKVER $STATIC $UNICODE --cxxflags $WXLIBS`";

WINDRES_FLAGS="`$wxconfig_EXEC $GTKVER $STATIC $UNICODE --cflags | sed 's/-m[[^ ]]*//g'`"
AC_SUBST(WINDRES_FLAGS)

AC_OUTPUT(
	Makefile
	zertz/Makefile
	dvonn/Makefile
	bloks/Makefile
	relax/Makefile
	common/Makefile
	skins/Makefile
	skins/eins/Makefile
	sounds/Makefile
	locale/Makefile
	locale/de/Makefile
	locale/de/LC_MESSAGES/Makefile
	help/Makefile
	)
