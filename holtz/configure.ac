AC_INIT(holtz.cpp)
AM_INIT_AUTOMAKE(holtz, 1.1.11)
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

AC_ARG_ENABLE(wxstatic,AC_HELP_STRING([--enable-wxstatic=VAL],
	[VAL=yes: Link WX libraries static; 
	 VAL=no: Link WX as shared library]),
	[wxstatic_Enabled="$enableval"])

AC_ARG_ENABLE(gtk1,AC_HELP_STRING([--enable-gtk1=VAL],
	[VAL=yes: call wx-config --gtk1 ...; 
	 VAL=no: use default wxGTK Version]),
	[gtk1_Enabled="$enableval"])

GTKVER=

if test "$gtk1_Enabled" = "yes"; then
  GTKVER=--gtk1
fi

if test "$wxstatic_Enabled" = "yes" ; then
  export LIBS="$LIBS `wx-config $GTKVER --static --libs`";
  export CXXFLAGS="$CXXFLAGS `wx-config $GTKVER --static --cxxflags`"
else
  export LIBS="$LIBS `wx-config $GTKVER --libs`";
  export CXXFLAGS="$CXXFLAGS `wx-config $GTKVER --cxxflags`"
fi

AC_OUTPUT(
	Makefile
	skins/Makefile
	skins/eins/Makefile
	sounds/Makefile
	locale/Makefile
	locale/de/Makefile
	locale/de/LC_MESSAGES/Makefile
	help/Makefile
	)
