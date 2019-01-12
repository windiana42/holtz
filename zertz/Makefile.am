INCLUDES = -I$(srcdir)/.. -I$(srcdir)

noinst_LIBRARIES = libzertz.a
libzertz_a_SOURCES = \
	zertz.hpp zertz.cpp wxzertz.hpp wxzertz.cpp \
	ai.hpp ai.cpp animations.hpp animations.cpp \
	dialogs.hpp dialogs.cpp manager.hpp manager.cpp \
	pbm.hpp pbm.cpp \
	network.hpp network.cpp bgp.hpp bgp.cpp

libzertz_a_CXXFLAGS= \
	-DDEFAULT_DATA_DIR=\"$(pkgdatadir)/\"
