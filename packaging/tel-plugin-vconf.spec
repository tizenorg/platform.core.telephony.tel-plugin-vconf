%define major 0
%define minor 1
%define patchlevel 91

Name:           tel-plugin-vconf
Version:        %{major}.%{minor}.%{patchlevel}
Release:        1
License:        Apache
Summary:        Telephony Vconf storage plugin
Group:          System/Libraries
Source0:        tel-plugin-vconf-%{version}.tar.gz
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
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DVERSION=$versionint \
#%if 0%{?model_build_feature_network_dsds}
#        -DTIZEN_FEATURE_DSDS=1 \
#%endif
#%if 0%{?model_build_feature_telephony_cdma}
#        -DTIZEN_FEATURE_CDMA=1 \
#%endif

make %{?_smp_mflags}

%post
/sbin/ldconfig

#Default Call Statistics
vconftool set -t int db/dnet/statistics/cellular/totalsnt 0 -i -f -s system::vconf_system
vconftool set -t int db/dnet/statistics/cellular/totalrcv 0 -i -f -s system::vconf_system
vconftool set -t int db/dnet/statistics/cellular/lastsnt 0 -i -f -s system::vconf_system
vconftool set -t int db/dnet/statistics/cellular/lastrcv 0 -i -f -s system::vconf_system
vconftool set -t int db/dnet/statistics/cellular/lastsnt2 0 -i -f -s system::vconf_system
vconftool set -t int db/dnet/statistics/cellular/totalrcv2 0 -i -f -s system::vconf_system
vconftool set -t int db/dnet/statistics/cellular/totalsnt2 0 -i -f -s system::vconf_system
vconftool set -t int db/dnet/statistics/cellular/lastrcv2 0 -i -f -s system::vconf_system

#General Telephony Informations
vconftool set -t bool memory/telephony/telephony_ready 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/tapi_state 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/daemon_load_count 0 -i -f -s system::vconf_system

#Dnet Informations
vconftool set -t int memory/dnet/state 0 -i -f -s system::vconf_system
vconftool set -t int memory/dnet/cellular 4 -i -f -s system::vconf_system
vconftool set -t int memory/dnet/packet_state 0 -i -f -s system::vconf_system
vconftool set -t int memory/dnet/state2 0 -i -f -s system::vconf_system

#Network Informations
vconftool set -t int memory/telephony/svc_type 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/svc_cs 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/svc_ps 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/svc_act 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/ps_type 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/rssi 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/svc_roam 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/plmn 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/lac 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/cell_id 0 -i -f -s system::vconf_system
vconftool set -t string memory/telephony/nw_name "" -i -f -s system::vconf_system
vconftool set -t string memory/telephony/nitz_zone 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/nitz_gmt 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/nitz_event_gmt 0 -i -f -s system::vconf_system
vconftool set -t bool db/telephony/flight_mode 0 -i -f -s system::vconf_system

#Call Informations
vconftool set -t int memory/telephony/call_state 0 -i -f -s system::vconf_system
vconftool set -t int db/telephony/call_forward_state 0 -i -f -s system::vconf_system
vconftool set -t int db/telephony/ss_cli_state 0 -i -f -s telephony_framework::vconf -g 6514
vconftool set -t int db/telephony/ss_cli_state2 0 -i -f -s telephony_framework::vconf -g 6514

#SIM informations
vconftool set -t int memory/telephony/pb_init 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/sim_slot 0 -i -f -s system::vconf_system
#vconf key for secondary sim slot
vconftool set -t int memory/telephony/sim_slot2 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/sim_slot_count -1 -i -f -s system::vconf_system
vconftool set -t string memory/telephony/spn "" -i -f -s telephony_framework::vconf
vconftool set -t int memory/telephony/spn_disp_condition 0 -i -f -s telephony_framework::vconf
vconftool set -t int memory/telephony/sim_status 255 -i -f -s telephony_framework::vconf
vconftool set -t string memory/telephony/cphs_operator_name_full "" -i -f -s telephony_framework::vconf
vconftool set -t string memory/telephony/cphs_operator_name_short "" -i -f -s telephony_framework::vconf
vconftool set -t int memory/telephony/sim_is_changed -1 -i -f -s telephony_framework::vconf
vconftool set -t int memory/telephony/sat_idle 0 -i -f -s system::vconf_system
vconftool set -t int memory/telephony/sat_state 0 -i -f -s system::vconf_system
vconftool set -t bool memory/private/telephony/modem_state 0 -i -f -s telephony_framework::vconf

#etc...
vconftool set -t int memory/telephony/zuhause_zone 0 -i -f -s system::vconf_system
vconftool set -t string memory/telephony/idle_text "" -i -f -s system::vconf_system
vconftool set -t int db/telephony/modem_always_on 2 -i -f -s system::vconf_system -g 6514

#dds
vconftool set -t int db/telephony/dualsim/default_data_service 0 -i -f -s system::vconf_system
vconftool set -t int db/telephony/dualsim/preferred_voice_subscription 1 -i -f -s system::vconf_system
vconftool set -t int db/telephony/dualsim/default_subscription 0 -i -f -s system::vconf_system
vconftool set -t bool db/telephony/dualsim/receive_incoming_call 0 -i -f -s system::vconf_system -g 6514

%postun -p /sbin/ldconfig

%install
%make_install
mkdir -p %{buildroot}%{_datadir}/license

%files
%manifest tel-plugin-vconf.manifest

%defattr(-,root,root,-)
#%doc COPYING
%{_libdir}/telephony/plugins/vconf-plugin*
%{_datadir}/license/tel-plugin-vconf
