INCLUDES = -I$(srcdir)/.. -I$(srcdir)

noinst_LIBRARIES = libbloks.a
libbloks_a_SOURCES = \
	bloks.hpp bloks.cpp wxbloks.hpp wxbloks.cpp \
	animations.hpp animations.cpp ai.hpp ai.cpp \
	dialogs.hpp dialogs.cpp manager.hpp manager.cpp \
	pbm.hpp pbm.cpp \
	network.hpp network.cpp bgp.hpp bgp.cpp

libbloks_a_CXXFLAGS= \
	-DDEFAULT_DATA_DIR=\"$(pkgdatadir)/\"

