Name:		account-parser
Summary:	Account Parser Library
Version:	0.1.0
Release:	0
Group:		Social & Content/Other
License:	Apache-2.0
Source0:	%{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(accounts-svc)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(libtzplatform-config)

%if "%{?profile}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%description
account parser C library of package manager to install account application.

%prep
%setup -q

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%ifarch %{ix86}
CXXFLAGS="$CXXFLAGS -D_OSP_DEBUG_ -D_SECURE_LOG -D_OSP_X86_ -D_OSP_EMUL_" cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DFULLVER=%{version} -DMAJORVER=${MAJORVER} -DSYSCONFDIR=%{_sysconfdir}
%else
CXXFLAGS="-O2 -g -pipe -Wall -fno-exceptions -Wformat -Wformat-security -Wl,--as-needed -fmessage-length=0 -march=armv7-a -mtune=cortex-a8 -mlittle-endian -mfpu=neon -mfloat-abi=softfp -D__SOFTFP__ -mthumb -Wa,-mimplicit-it=thumb -funwind-tables -D_OSP_DEBUG_ -D_SECURE_LOG -D_OSP_ARMEL_" cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DFULLVER=%{version} -DMAJORVER=${MAJORVER} -DSYSCONFDIR=%{_sysconfdir}
%endif

# Call make instruction with smp support
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp %{_builddir}/%{name}-%{version}/LICENSE.APLv2  %{buildroot}/usr/share/license/%{name}

mkdir -p %{buildroot}/lib

%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest account-parser.manifest
/usr/share/license/%{name}
%{_sysconfdir}/package-manager/parserlib/libaccount.so*
