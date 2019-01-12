SUBDIRS=skins sounds locale help common bloks dvonn zertz relax

bin_PROGRAMS=		holtz

holtz_SOURCES= \
	util.cpp util.hpp \
	wxutil.cpp wxutil.hpp \
	wxmain.cpp wxmain.hpp \
	wxholtz.cpp wxholtz.hpp \
	line_net.cpp line_net.hpp \
	msg_net_templ.cpp msg_net.hpp

if INCLUDE_RC_FILE
holtz_SOURCES += holtz.rc
endif

holtz_LDADD = bloks/libbloks.a dvonn/libdvonn.a zertz/libzertz.a relax/librelax.a

holtz_CXXFLAGS= \
	-DDEFAULT_DATA_DIR=\"$(pkgdatadir)/\"

EXTRA_DIST=	\
	Rules.german Rules.english holtz.rc holtz.ico holtz.sln holtz.vcproj icon.xpm holtz.spec 

.rc.o:
if INCLUDE_RC_FILE
	$(WINDRES) -I$(srcdir) $(WINDRES_FLAGS) -o $@ $<	
else
	echo "No windows evironment, so ignore resource file $<"
endif
