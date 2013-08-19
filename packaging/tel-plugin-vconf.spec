%define major 0
%define minor 1
%define patchlevel 32
Name:       tel-plugin-vconf
Summary:    Telephony Vconf storage plugin
Version:    %{major}.%{minor}.%{patchlevel}
Release:    1
Group:      System/Libraries
License:    Apache
Source0:    tel-plugin-vconf-%{version}.tar.gz
Source1001: 	tel-plugin-vconf.manifest
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gthread-2.0)
BuildRequires:  pkgconfig(tcore)
BuildRequires:  pkgconfig(dlog)

%description
Telephony Vconf storage plugin

%prep
%setup -q
cp %{SOURCE1001} .

%build
versionint=$[%{major} * 1000000 + %{minor} * 1000 + %{patchlevel}]
%cmake . -DVERSION=$versionint
make %{?jobs:-j%jobs}

%post
/sbin/ldconfig

#Default Call Statistics
vconftool set -t int db/dnet/statistics/cellular/totalsnt 0 -i -f
vconftool set -t int db/dnet/statistics/cellular/totalrcv 0 -i -f
vconftool set -t int db/dnet/statistics/cellular/lastsnt 0 -i -f
vconftool set -t int db/dnet/statistics/cellular/lastrcv 0 -i -f

##setting vconf key##
vconftool set -t int memory/dnet/state 0 -i
vconftool set -t int memory/dnet/packet_state 0 -i
vconftool set -t int memory/dnet/cellular 4 -i
vconftool set -t int memory/telephony/svc_type 0 -i -f
vconftool set -t int memory/telephony/ps_type 0 -i -f
vconftool set -t int memory/telephony/rssi 0 -i -f
vconftool set -t int memory/telephony/sim_slot 0 -i -f
vconftool set -t int memory/telephony/svc_roam 0 -i -f
vconftool set -t int memory/telephony/plmn 0 -i -f
vconftool set -t int memory/telephony/lac 0 -i -f
vconftool set -t int memory/telephony/cell_id 0 -i -f
vconftool set -t int memory/telephony/svc_cs 0 -i -f
vconftool set -t int memory/telephony/svc_ps 0 -i -f
vconftool set -t int memory/telephony/pb_init 0 -i -f
vconftool set -t int memory/telephony/call_state 0 -i -f
vconftool set -t int db/telephony/call_forward_state 0 -i -f
vconftool set -t int memory/telephony/tapi_state 0 -i -f
vconftool set -t int memory/telephony/spn_disp_condition 0 -i -f
vconftool set -t int memory/telephony/zuhause_zone 0 -i -f
vconftool set -t string memory/telephony/spn "" -i -f
vconftool set -t string memory/telephony/nw_name "" -i -f
vconftool set -t string memory/telephony/imei "" -i -f
vconftool set -t string memory/telephony/szSubscriberNumber "" -i -f
vconftool set -t string memory/telephony/szSWVersion "" -i -f
vconftool set -t string memory/telephony/szHWVersion "" -i -f
vconftool set -t string db/private/tel-plugin-vconf/imsi "" -f
vconftool set -t int db/telephony/emergency 0 -i -f
vconftool set -t bool memory/telephony/telephony_ready 0 -i -f
vconftool set -t int memory/telephony/nitz_gmt 0 -i -f
vconftool set -t int memory/telephony/nitz_event_gmt 0 -i -f
vconftool set -t string memory/telephony/nitz_zone 0 -i -f
vconftool set -t int memory/telephony/svc_act 0 -i -f
vconftool set -t bool db/telephony/flight_mode 0 -i -f

%postun -p /sbin/ldconfig

%install
%make_install
mkdir -p %{buildroot}/usr/share/license

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/telephony/plugins/vconf-plugin*
/usr/share/license/tel-plugin-vconf
