SUBDIRS=skins sounds locale help zertz dvonn

bin_PROGRAMS=		holtz

holtz_SOURCES= \
	util.cpp util.hpp \
	wxmain.cpp wxmain.hpp \
	wxholtz.cpp wxholtz.hpp \
	line_net.cpp line_net.hpp

holtz_LDADD = dvonn/libdvonn.a zertz/libzertz.a

holtz_CXXFLAGS= \
	-DDEFAULT_DATA_DIR=\"$(pkgdatadir)/\"
#	-DDEFAULT_DATA_DIR=wxT\(\"$(pkgdatadir)/\"\)

EXTRA_DIST=	\
	Rules.german Rules.english holtz.rc holtz.ico holtz.dsp holtz.dsw icon.xpm holtz.spec 

