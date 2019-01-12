INCLUDES = -I$(srcdir)/.. -I$(srcdir)

noinst_LIBRARIES = libdvonn.a
libdvonn_a_SOURCES = \
	dvonn.hpp dvonn.cpp wxdvonn.hpp wxdvonn.cpp \
	animations.hpp animations.cpp ai.hpp ai.cpp \
	dialogs.hpp dialogs.cpp manager.hpp manager.cpp \
	pbm.hpp pbm.cpp littlegolem.hpp littlegolem.cpp \
	network.hpp network.cpp bgp.hpp bgp.cpp

libdvonn_a_CXXFLAGS= \
	-DDEFAULT_DATA_DIR=\"$(pkgdatadir)/\"

