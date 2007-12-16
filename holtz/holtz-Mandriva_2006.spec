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
Version: 1.2.2
Release: %rel

Summary: Holtz is an implementation of the two player abstract board games Zertz and Dvonn
License: GPL
Group: Recreation/
Vendor: Martin Trautmann <martintrautmann@gmx.de>
Packager: Martin Trautmann <martintrautmann@gmx.de>
Url: http://holtz.sourceforge.net/

Source: %name-%version.tar.gz

Prefix: %_prefix
BuildRoot: %_tmppath/%name-%version-root

%if %{semistatic}
#I am not sure about precise requirements of wxGTK-static
Requires:      gtk+ >= 1.2.7 gettext
BuildRequires: wxGTK >= 2.8.4 wxGTK-devel gcc-c++
#wxGTK-static has to be manually compiled
%else
#-- OpenSUSE 10.3  
#Requires:      wxGTK >= 2.8.4
#BuildRequires: wxGTK-devel >= 2.8.4 gcc-c++
#-- OpenSUSE 10.2 - OpenSUSE Build Service
#Requires:      wxGTK >= 2.6.1  
#BuildRequires: wxGTK-devel >= 2.6.1 gcc-c++  
#%#define BACKWARDCOMPATIBILITY "--enable-wx_2_6"
#-- Feodora 8 - OpenSUSE Build Service
#Requires:      wxGTK >= 2.8.4  
#BuildRequires: wxGTK-devel >= 2.8.4 gcc-c++ lynx  
#-- Mandriva 2007 - OpenSUSE Build Service
#Requires:      wxGTK >= 2.6.0  
#BuildRequires: wxGTK-devel >= 2.6.0 gcc-c++  
#%#define BACKWARDCOMPATIBILITY "--enable-wx_2_6"  
#%#define UNICODE "--disable-unicode"  
#-- Mandriva 2006 - OpenSUSE Build Service
Requires:      wxGTK >= 2.6.0  
BuildRequires: wxGTK-devel >= 2.6.0 gcc-c++  
%define BACKWARDCOMPATIBILITY "--enable-wx_2_6"  
%define UNICODE "--disable-unicode"  
#-- Debian Etch - OpenSUSE Build Service
#Requires:      libwxgtk2.6
#BuildRequires: libwxgtk2.6-dev gcc-c++ hostname alien
#%#define BACKWARDCOMPATIBILITY "--enable-wx_2_6"
#--
%endif

Provides: holtz

%description
Holtz is an implementation of the two player abstract board games Zertz and Dvonn from the gipf probject (www.gipf.com).
Zertz is about placing and collecting stones, making sacrifices, and a continuously shriking board. 
Dvonn is about controlling stacks of stones which can jump on other stacks to capture them and keeping 
contact to the three dvonn stones.

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
%prefix/share/locale/*/LC_MESSAGES/
%prefix/share/holtz/skins/*
%prefix/share/holtz/sounds/*
%prefix/share/holtz/help/*

#%changelog
#* first version
#- sdaf
