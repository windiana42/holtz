SUBDIRS=skins sounds locale help

bin_PROGRAMS=		holtz

holtz_SOURCES= \
	holtz.cpp wxholtz.cpp dialogs.cpp manager.cpp network.cpp ai.cpp animations.cpp util.cpp pbm.cpp  \
	holtz.hpp wxholtz.hpp dialogs.hpp manager.hpp network.hpp ai.hpp animations.hpp util.hpp pbm.hpp  \
	wxmain.cpp \
	wxmain.hpp

holtz_CXXFLAGS= \
	-DDEFAULT_DATA_DIR=\"$(pkgdatadir)/\"
#	-DDEFAULT_DATA_DIR=wxT\(\"$(pkgdatadir)/\"\)

EXTRA_DIST=	\
	Rules.german Rules.english holtz.rc holtz.ico holtz.dsp holtz.dsw icon.xpm holtz.spec 

