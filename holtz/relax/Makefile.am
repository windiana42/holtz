INCLUDES = -I$(srcdir)/.. -I$(srcdir)

bin_PROGRAMS=		relax

noinst_LIBRARIES = librelax.a
librelax_a_SOURCES = \
	relax.hpp relax.cpp \
	wxrelax.hpp wxrelax.cpp \
	dialogs.hpp dialogs.cpp manager.hpp manager.cpp \
	network.hpp network.cpp bgp.hpp bgp.cpp \
	ai.hpp ai.cpp animations.hpp animations.cpp \
	pbm.hpp pbm.cpp 

librelax_a_CXXFLAGS= \
	-DDEFAULT_DATA_DIR=\"$(pkgdatadir)/\"

relax_SOURCES = main.cpp ../util.hpp ../util.cpp
relax_LDADD = librelax.a