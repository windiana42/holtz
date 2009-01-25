AC_INIT(wxmain.cpp)
AM_INIT_AUTOMAKE(holtz, 1.3.0)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

#AC_CANONICAL_HOST

WXLIBS=
#I thought this would be right, but it actually doesn't work:
#WXLIBS=adv # this adds additional libs (ex: for sound in the advanced package of wxWidgets)

case $OSTYPE in #$OSTYPE replaces former $host_os
*cygwin* )
	echo "Build System = Cygwin:";
	WXLIBS=; # adv doesn't seem to do good for cygwin version
	AM_CONDITIONAL([INCLUDE_RC_FILE],[test 1 -eq 1]);;
#	export CXXFLAGS="$CXXFLAGS -mno-cygwin -D__CYGWIN__"; # set true
#	export LIBS="$LIBS -lwinmm";
#	AC_CHECK_LIB(winmm, PlaySound);;
*mingw*  ) 
	echo "Build System = MinGW"; 
	AM_CONDITIONAL([INCLUDE_RC_FILE],[test 1 -eq 1]);;    # set true
#	export LIBS="$LIBS -lwinmm";
#	AC_CHECK_LIB(winmm, PlaySound);;	   
* ) 
	AM_CONDITIONAL([INCLUDE_RC_FILE],[test 0 -eq 1]);;    # set false
#	AC_CHECK_HEADER( unistd.h, export CXXFLAGS="$CXXFLAGS -D__UNIX__" );;
esac

wx_2_6_Enabled="no"
wxstatic_Enabled="no"
unicode_Enabled="yes"

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

echo "wx-config $GTKVER $STATIC $UNICODE --libs $WXLIBS";
export LIBS="$LIBS `wx-config $GTKVER $STATIC $UNICODE --libs $WXLIBS`";
export CXXFLAGS="$CXXFLAGS $BACKCOMPATIBILITY `wx-config $GTKVER $STATIC $UNICODE --cxxflags $WXLIBS`";

AC_OUTPUT(
	Makefile
	zertz/Makefile
	dvonn/Makefile
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
