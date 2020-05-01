# set this to 1 if you want to build semistatic rpm
%define        semistatic      0
%{?_with_semistatic: %{expand: %%define semistatic 1}}
%{?_without_semistatic: %{expand: %%define semistatic 0}}

%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
  
# default settings:  
%define BACKWARDCOMPATIBILITY ""  
%define UNICODE "--enable-unicode"  

Name: holtz
Version: 1.4.1
Release: %rel

Summary: Holtz is an implementation of the abstract board games Zertz and Dvonn
License: GPLv2
Group: Amusements/Games/Board/Other
Vendor: Martin Trautmann <martintrautmann@gmx.de>
Url: http://holtz.sourceforge.net/

Source: %name-%version.tar.gz

Prefix: %_prefix
BuildRoot: %_tmppath/%name-%version-root

## distribution specific settings

## SUSE
%if 0%{?suse_version}
%if 0%{?suse_version} >= 1140
%if %{semistatic}
#I am not sure about precise requirements of wxGTK-static
Requires:      gtk+ >= 1.2.7 gettext
BuildRequires: wxWidgets >= 3.0.0 wxWidgets-devel gcc-c++ boost-devel
#wxGTK-static has to be manually compiled
%else
Requires:      wxWidgets >= 3.0.0
BuildRequires: wxWidgets-devel >= 3.0.0 gcc-c++ boost-devel
%endif
%else
%if %{semistatic}
#I am not sure about precise requirements of wxGTK-static
Requires:      gtk+ >= 1.2.7 gettext
BuildRequires: wxGTK >= 3.0.0 wxGTK-devel gcc-c++ boost-devel
#wxGTK-static has to be manually compiled
%else
Requires:      wxGTK >= 3.0.0
BuildRequires: wxGTK-devel >= 3.0.0 gcc-c++ boost-devel
%endif
%endif

## Fedora
%else
%if 0%{?fedora}
%if %{semistatic}
#I am not sure about precise requirements of wxGTK-static
Requires:      gtk+ >= 1.2.7 gettext
BuildRequires: wxGTK >= 3.0.0 wxGTK-devel gcc-c++ boost-devel
#wxGTK-static has to be manually compiled
%else
Requires:      wxGTK >= 3.0.0
BuildRequires: wxGTK-devel >= 3.0.0 gcc-c++ boost-devel
%endif

## Mandriva
%else
%if 0%{?mdkversion}
#%if 0%{?mdkversion} < 201100
%define UNICODE "--disable-unicode"
#%endif  
%if %{semistatic}
#I am not sure about precise requirements of wxGTK-static
Requires:      gtk+ >= 1.2.7 gettext
BuildRequires: wxGTK >= 3.0.0 wxGTK-devel gcc-c++ boost-devel
#wxGTK-static has to be manually compiled
%else
Requires:      wxGTK >= 3.0.0
BuildRequires: wxGTK-devel >= 3.0.0 gcc-c++ boost-devel
%endif

## Redhat / Centos
%else
%if 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}
%if %{semistatic}
#I am not sure about precise requirements of wxGTK-static
Requires:      gtk+ >= 1.2.7 gettext
BuildRequires: wxWidgets >= 3.0.0 wxWidgets-devel gcc-c++ boost-devel
#wxGTK-static has to be manually compiled
%else
Requires:      wxWidgets >= 3.0.0
BuildRequires: wxWidgets-devel >= 3.0.0 gcc-c++ boost-devel
%endif

## any other (just speculation what might be most common)
%else
%if %{semistatic}
#I am not sure about precise requirements of wxGTK-static
Requires:      gtk+ >= 1.2.7 gettext
BuildRequires: wxGTK >= 3.0.0 wxGTK-devel gcc-c++ boost-devel
#wxGTK-static has to be manually compiled
%else
Requires:      wxGTK >= 3.0.0
BuildRequires: wxGTK-devel >= 3.0.0 gcc-c++ boost-devel
%endif
# if Redhat else any other
%endif 
# if Mandriva else Redhat+
%endif
# if Fedora else Mandriva+
%endif
# if Suse else Fedora+
%endif

Provides: holtz-%version

%description
Holtz is an implementation of the two player abstract board games Zertz and Dvonn from the gipf probject (www.gipf.com). Additionally it has two more games called Relax and Bloks.
Zertz is about placing and collecting stones, making sacrifices, and a continuously shriking board.
Dvonn is about controlling stacks of stones which can jump on other stacks to capture them and keeping
contact to the three dvonn stones. Version 1.3.0 added a third game called Relax which resembles the game
"Take it Easy". Version 1.4.0 added a forth game called Bloks which is about placing oddly shaped pieces on an increasingly crowded board. 

%prep
%setup -q
#perl -pi -e 's|\${prefix}|%prefix|' README
#perl -pi -e 's|PREFIX|%prefix|' doc/FAQ

%build

if [ ! -x configure ]; then
	CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $ARCH_FLAGS --prefix=%prefix
fi
%if %{semistatic}
	CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure --enable-wxstatic %BACKWARDCOMPATIBILITY %UNICODE $ARCH_FLAGS --prefix=%prefix
%else
	CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure --disable-wxstatic %BACKWARDCOMPATIBILITY %UNICODE $ARCH_FLAGS --prefix=%prefix
%endif

if [ -n "$SMP" ]; then
	make -j$SMP "MAKE=make -j$SMP"
else
	make
fi

%install
rm -rf $RPM_BUILD_ROOT
#mkdir -p -m 755 $RPM_BUILD_ROOT%prefix/{{include,lib}/%lib_name}
make install-strip prefix=$RPM_BUILD_ROOT%prefix

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,755)
%doc AUTHORS COPYING README NEWS ChangeLog TODO Rules.german Rules.english
%prefix/bin/*
%prefix/share/locale/*/LC_MESSAGES/*.mo
%prefix/share/holtz/skins/*
%prefix/share/holtz/sounds/*
%prefix/share/holtz/help/*
%prefix/share/holtz/

#%changelog
#* 1.4.0
#- added new game bloks
#- fixed rc file compilation for crosscompile to linux
#* first version
#- added support for OpenSuSE >= 11.4
#- added support for Fedora 14/15
#- added support for Mandriva
#- CentOS 5, RHEL 5 and possibly RHEL 6 have no wxGTK 2.8 packages (I didn't find any)
#- CentOS 6 is not available in build service, yet
