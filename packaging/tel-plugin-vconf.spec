%define major 0
%define minor 2
%define patchlevel 4

Name:           tel-plugin-vconf
Version:        %{major}.%{minor}.%{patchlevel}
Release:        1
License:        Apache-2.0
Summary:        Telephony Vconf storage plugin
Group:          System/Libraries
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  cmake
#BuildRequires:	model-build-features
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(tcore)
BuildRequires:  pkgconfig(vconf)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Telephony Vconf storage plugin

%prep
%setup -q

%build
versionint=$[%{major} * 1000000 + %{minor} * 1000 + %{patchlevel}]
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} \
	-DLIB_INSTALL_DIR=%{_libdir} \
	-DVERSION=$versionint \
#%if 0%{?model_build_feature_telephony_cdma}
#	-DTIZEN_FEATURE_CDMA=1 \
#%endif
#%if 0%{?prepaid_sim_apn_support}
#	-DPREPAID_SIM_APN_SUPPORT=1 \
#%endif

make %{?_smp_mflags}

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%install
%make_install
mkdir -p %{buildroot}%{_datadir}/license

%files
%manifest tel-plugin-vconf.manifest

%defattr(644,system,system,-)
#%doc COPYING
%{_libdir}/telephony/plugins/vconf-plugin*
%{_datadir}/license/tel-plugin-vconf
