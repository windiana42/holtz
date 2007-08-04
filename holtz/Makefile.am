SUBDIRS=skins sounds locale help common zertz dvonn

bin_PROGRAMS=		holtz

holtz_SOURCES= \
	util.cpp util.hpp \
	wxmain.cpp wxmain.hpp \
	wxholtz.cpp wxholtz.hpp \
	line_net.cpp line_net.hpp \
	msg_net_templ.cpp msg_net.hpp

if INCLUDE_RC_FILE
holtz_SOURCES += holtz.rc
endif

holtz_LDADD = dvonn/libdvonn.a zertz/libzertz.a

holtz_CXXFLAGS= \
	-DDEFAULT_DATA_DIR=\"$(pkgdatadir)/\"
#	-DDEFAULT_DATA_DIR=wxT\(\"$(pkgdatadir)/\"\)

EXTRA_DIST=	\
	Rules.german Rules.english holtz.rc holtz.ico holtz.sln holtz.vcproj icon.xpm holtz.spec 

.rc.o:
	windres -I$(srcdir) `wx-config --cflags` -o $@ $<	
