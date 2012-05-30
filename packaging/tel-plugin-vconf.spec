#sbs-git:slp/pkgs/t/tel-plugin-vconf
Name:       tel-plugin-vconf
Summary:    Telephony Vconf storage plugin
Version: 0.1.7
Release:    1
Group:      System/Libraries
License:    Apache
Source0:    tel-plugin-vconf-%{version}.tar.gz
Source1001: packaging/tel-plugin-vconf.manifest 
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

%build
cp %{SOURCE1001} .
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%post
/sbin/ldconfig

#Default Call Statistics
vconftool set -t int db/dnet/statistics/cellular/totalsnt "0"
vconftool set -t int db/dnet/statistics/cellular/totalrcv "0"
vconftool set -t int db/dnet/statistics/cellular/lastsnt "0"
vconftool set -t int db/dnet/statistics/cellular/lastrcv "0"

##setting vconf key##
vconftool set -t int memory/dnet/state 0 -i
vconftool set -t int memory/dnet/cellular 4 -i
vconftool set -t int memory/telephony/svc_type 0 -i -f
vconftool set -t int memory/telephony/rssi 0 -i -f
vconftool set -t int memory/telephony/sim_slot 0 -i -f
vconftool set -t int memory/telephony/svc_roam 0 -i -f
vconftool set -t int memory/telephony/event_system_ready 0 -i -f
vconftool set -t int memory/telephony/plmn 0 -i -f
vconftool set -t int memory/telephony/lac 0 -i -f
vconftool set -t int memory/telephony/cell_id 0 -i -f
vconftool set -t int memory/telephony/svc_cs 0 -i -f
vconftool set -t int memory/telephony/svc_ps 0 -i -f
vconftool set -t int memory/telephony/zone_type 0 -i -f
vconftool set -t int memory/telephony/sim_init 0 -i -f
vconftool set -t int memory/telephony/sim_chv 255
vconftool set -t int memory/telephony/pb_init 0 -i -f
vconftool set -t int memory/telephony/call_state 0 -i -f
vconftool set -t int memory/telephony/call_forward_state 0 -i -f
vconftool set -t int memory/telephony/tapi_state 0 -i -f
vconftool set -t int memory/telephony/spn_disp_condition 0 -i -f
vconftool set -t int memory/telephony/sat_idle 0 -i -f
vconftool set -t int memory/telephony/sat_state 0 -i -f
vconftool set -t int memory/telephony/zuhause_zone 0 -i -f
vconftool set -t int memory/telephony/low_battery 0 -i -f
vconftool set -t int memory/telephony/bIMSIFactoryProcess 0 -i -f
vconftool set -t string memory/telephony/idle_text "" -i -f
vconftool set -t string memory/telephony/spn "" -i -f
vconftool set -t string memory/telephony/nw_name "" -i -f
vconftool set -t string memory/telephony/imei "" -i -f
vconftool set -t string memory/telephony/szSubscriberNumber "" -i -f
vconftool set -t string memory/telephony/szSubscriberAlpha "" -i -f
vconftool set -t string memory/telephony/szSWVersion "" -i -f
vconftool set -t string memory/telephony/szHWVersion "" -i -f
vconftool set -t string memory/telephony/szCalDate "" -i -f
vconftool set -t string db/telephony/imsi "" -f
vconftool set -t int db/telephony/emergency 0 -i -f

%postun -p /sbin/ldconfig

%install
rm -rf %{buildroot}
%make_install

%files
%manifest tel-plugin-vconf.manifest
%defattr(-,root,root,-)
#%doc COPYING
%{_libdir}/telephony/plugins/vconf-plugin*
