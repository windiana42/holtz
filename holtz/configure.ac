AC_INIT(wxmain.cpp)
AM_INIT_AUTOMAKE(holtz, 1.2.0)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

#AC_CANONICAL_HOST

#case $host_os in
#*cygwin* ) export CXXFLAGS="$CXXFLAGS -mno-cygwin -D__CYGWIN__";
#	   export LIBS="$LIBS -lwinmm";
#	   AC_CHECK_LIB(winmm, PlaySound);;
#*mingw*  ) export LIBS="$LIBS -lwinmm";
#	   AC_CHECK_LIB(winmm, PlaySound);;	   
#       * ) AC_CHECK_HEADER( unistd.h, export CXXFLAGS="$CXXFLAGS -D__UNIX__" );;
#esac

wxstatic_Enabled="no"
unicode_Enabled="yes"

AC_ARG_ENABLE(wxstatic,AC_HELP_STRING([--enable-wxstatic=VAL],
	[VAL=yes: Link WX libraries static; 
	 VAL=no: Link WX as shared library]),
	[wxstatic_Enabled="$enableval"])

AC_ARG_ENABLE(unicode,AC_HELP_STRING([--enable-unicode=VAL],
	[VAL=yes: call wx-config --unicode ...; 
	 VAL=no: use default wxGTK Version]),
	[unicode_Enabled="$enableval"])

GTKVER=
STATIC=
UNICODE=

if test "$wxstatic_Enabled" = "yes" ; then
  STATIC=--static=yes
else
  STATIC=--static=no
fi

if test "$unicode_Enabled" = "yes" ; then
  UNICODE=--unicode=yes
else
  UNICODE=--unicode=no
fi

export LIBS="$LIBS `wx-config $GTKVER $STATIC $UNICODE --libs`";
export CXXFLAGS="$CXXFLAGS `wx-config $GTKVER $STATIC $UNICODE --cxxflags`"

AC_OUTPUT(
	Makefile
	zertz/Makefile
	dvonn/Makefile
	common/Makefile
	skins/Makefile
	skins/eins/Makefile
	sounds/Makefile
	locale/Makefile
	locale/de/Makefile
	locale/de/LC_MESSAGES/Makefile
	help/Makefile
	)
