SUBDIRS=skins sounds eins locale help

bin_PROGRAMS=		holtz

holtz_SOURCES= \
	holtz.cpp wxholtz.cpp dialogs.cpp network.cpp ai.cpp animations.cpp util.cpp \
	holtz.hpp wxholtz.hpp dialogs.hpp network.hpp ai.hpp animations.hpp util.hpp

holtz_CXXFLAGS= \
	`wx-config --cxxflags` -DDEFAULT_DATA_DIR=\"$(pkgdatadir)/\"

EXTRA_DIST=	\
	Rules.german Rules.english holtz.rc holtz.ico holtz.dsp holtz.dsw icon.xpm holtz.spec 

