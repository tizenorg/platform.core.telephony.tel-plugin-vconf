/*
 * tel-plugin-vconf
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Ja-young Gu <jygu@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>

#include <glib.h>
#include <vconf.h>

#include <tcore.h>
#include <server.h>
#include <plugin.h>
#include <storage.h>
#include <co_network.h>

#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION 1
#endif

#ifndef VCONFKEY_SETAPPL_STATE_DATA_ROAMING_APP_STATUS
#define VCONFKEY_SETAPPL_STATE_DATA_ROAMING_APP_STATUS VCONFKEY_SETAPPL_PREFIX"/data_roaming_app/status"
#endif
#define VCONFKEY_PRIVATE_TELEPHONY_NCK_UNLOCK_COUNT 			"memory/private/telephony/nck_unlock_count"
#define VCONFKEY_PRIVATE_TELEPHONY_NO_SIM_POPUP_CHECKBOX		"db/private/telephony/no_sim_popup_checkbox"
#define VCONFKEY_PRIVATE_TELEPHONY_DATA_ROAMING_POPUP_CHECKBOX	"db/private/telephony/data_roaming_popup_checkbox"
#define VCONFKEY_PRIVATE_TELEPHONY_MODEM_ON_COUNT				"memory/private/telephony/modem_on_count"

struct vconf_plugin_user_data {
	gboolean b_get_nck_retry_count;

	enum modem_state last_modem_power_state;
	gboolean last_flight_mode_state;
	long last_modem_state_timestamp;
};

#ifndef VCONFKEY_SETAPPL_MOBILE_DATA_POPUP_DONE
#define VCONFKEY_SETAPPL_MOBILE_DATA_POPUP_DONE VCONFKEY_SETAPPL_PREFIX"/mobile_data_popup"
#endif

#define VCONFKEY_TELEPHONY_DB_DEFAULT_DATA_SUBS  "db/telephony/dualsim/default_data_service"
#define VCONFKEY_TELEPHONY_PREFERRED_VOICE_SUBSCRIPTION	"db/telephony/dualsim/preferred_voice_subscription"
#define VCONFKEY_TELEPHONY_DB_DEFAULT_SUBS	"db/telephony/dualsim/default_subscription"

#define VCONFKEY_WECONN_ALL_CONNECTED	"memory/private/weconn/all_connected" // True/False
#define VCONFKEY_SAP_CONNECTION_TYPE	"memory/private/sap/conn_type" // SAPInterface.h
#define VCOKFKEY_TELEPHONY_MODEM_STATE  "memory/private/telephony/modem_state"

static void reset_vconf();

static TcoreStorageDispatchCallback callback_dispatch;

static gboolean __vconf_check_process_hook_callback(CoreObject *co)
{
	const char *cp_name;

	cp_name = tcore_server_get_cp_name_by_plugin(tcore_object_ref_plugin(co));
	dbg("CP name: [%s]", cp_name);

	if (cp_name == NULL)
		return FALSE;

	return g_str_has_suffix(cp_name, "0");
}

static void __vconf_write_power_status_log(struct vconf_plugin_user_data *ud, enum modem_state state)
{
	struct sysinfo sys_info;

	if (ud == NULL)
		return;

	if (0 != sysinfo(&sys_info)) {
		err("sysinfo failed.");
	}

	if (state == MODEM_STATE_ONLINE) {
		if (ud->last_modem_power_state == MODEM_STATE_LOW) {
			int count = 0;
			if (0 == vconf_get_int(VCONFKEY_PRIVATE_TELEPHONY_MODEM_ON_COUNT, &count)) {
				count++;
				if (0 != vconf_set_int(VCONFKEY_PRIVATE_TELEPHONY_MODEM_ON_COUNT, count)) {
					err("vconf_set_int failed.");
				}
			} else {
				err("vconf_get_int failed.");
			}
			msg("[MODEM ON/OFF] MODEM LOW => ON in %d secs. (modem_on_count[%d] after boot-up. (uptime %ld secs))",
					sys_info.uptime - ud->last_modem_state_timestamp, count, sys_info.uptime);
			ud->last_modem_power_state = state;
			ud->last_modem_state_timestamp = sys_info.uptime;
		}
	} else if (state == MODEM_STATE_LOW) {
		if (ud->last_modem_power_state == MODEM_STATE_ONLINE) {
			int count = 0;
			if (0 != vconf_get_int(VCONFKEY_PRIVATE_TELEPHONY_MODEM_ON_COUNT, &count)) {
				err("vconf_get_int failed.");
			}
			msg("[MODEM ON/OFF] MODEM ON => LOW in %d secs. (modem_on_count=[%d] after boot-up(uptime %ld secs))",
					sys_info.uptime - ud->last_modem_state_timestamp, count, sys_info.uptime);
			ud->last_modem_power_state = state;
			ud->last_modem_state_timestamp = sys_info.uptime;
		}
	}
}

static void __vconf_check_and_set_int(const char *in_key, const int intval)
{
	int current;
	vconf_get_int(in_key, &current);
	if (current != intval) {
		vconf_set_int(in_key, intval);
	}
}

static void __vconf_check_and_set_str(const char *in_key, const char *strval)
{
	char *current = vconf_get_str(in_key);

	if(current) {
		if(strval && strcmp(current, strval)) {
			vconf_set_str(in_key, strval);
		}
		free(current);
	}
	else {
		vconf_set_str(in_key, strval);
	}
}

static const gchar* convert_strgkey_to_vconf(enum tcore_storage_key key)
{
	switch (key) {
		case STORAGE_KEY_TELEPHONY_PLMN:
			return VCONFKEY_TELEPHONY_PLMN;
		case STORAGE_KEY_TELEPHONY_LAC:
			return VCONFKEY_TELEPHONY_LAC;
		case STORAGE_KEY_TELEPHONY_CELLID:
			return VCONFKEY_TELEPHONY_CELLID;
		case STORAGE_KEY_TELEPHONY_SVCTYPE:
			return VCONFKEY_TELEPHONY_SVCTYPE;
		case STORAGE_KEY_TELEPHONY_SVC_CS:
			return VCONFKEY_TELEPHONY_SVC_CS;
		case STORAGE_KEY_TELEPHONY_SVC_PS:
			return VCONFKEY_TELEPHONY_SVC_PS;
		case STORAGE_KEY_TELEPHONY_SVC_ROAM:
			return VCONFKEY_TELEPHONY_SVC_ROAM;
		case STORAGE_KEY_TELEPHONY_SIM_PB_INIT:
			return VCONFKEY_TELEPHONY_SIM_PB_INIT;
		case STORAGE_KEY_TELEPHONY_CALL_FORWARD_STATE:
			return VCONFKEY_TELEPHONY_CALL_FORWARD_STATE;
		case STORAGE_KEY_TELEPHONY_TAPI_STATE:
			return VCONFKEY_TELEPHONY_TAPI_STATE;
		case STORAGE_KEY_TELEPHONY_SPN_DISP_CONDITION:
			return VCONFKEY_TELEPHONY_SPN_DISP_CONDITION;
		case STORAGE_KEY_TELEPHONY_RSSI:
			return VCONFKEY_TELEPHONY_RSSI;
		case STORAGE_KEY_TELEPHONY_READY:
			return VCONFKEY_TELEPHONY_READY;
		case STORAGE_KEY_TELEPHONY_SIM_SLOT:
			return VCONFKEY_TELEPHONY_SIM_SLOT;
#ifdef TIZEN_FEATURE_DSDS
		case STORAGE_KEY_TELEPHONY_SIM_SLOT2:
			return VCONFKEY_TELEPHONY_SIM_SLOT2;
#endif
		case STORAGE_KEY_TELEPHONY_SIM_SLOT_COUNT:
			return VCONFKEY_TELEPHONY_SIM_SLOT_COUNT;
		case STORAGE_KEY_PM_STATE:
			return VCONFKEY_PM_STATE;
		case STORAGE_KEY_PACKET_SERVICE_STATE:
			return VCONFKEY_DNET_STATE;
#ifdef TIZEN_FEATURE_DSDS
		case STORAGE_KEY_PACKET_SERVICE_STATE2:
			return VCONFKEY_DNET_STATE2;
#endif
		case STORAGE_KEY_PACKET_INDICATOR_STATE:
			return VCONFKEY_PACKET_STATE;
		case STORAGE_KEY_3G_ENABLE:
			return VCONFKEY_3G_ENABLE;
		case STORAGE_KEY_TELEPHONY_DUALSIM_DEFAULT_DATA_SERVICE_INT:
			return VCONFKEY_TELEPHONY_DB_DEFAULT_DATA_SUBS;
		case STORAGE_KEY_SETAPPL_STATE_DATA_ROAMING_BOOL:
			return VCONFKEY_SETAPPL_STATE_DATA_ROAMING_BOOL;
		case STORAGE_KEY_TELEPHONY_NWNAME:
			return VCONFKEY_TELEPHONY_NWNAME;
		case STORAGE_KEY_TELEPHONY_SPN_NAME:
			return VCONFKEY_TELEPHONY_SPN_NAME;
		case STORAGE_KEY_CELLULAR_STATE:
			return VCONFKEY_NETWORK_CELLULAR_STATE;
		case STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV:
			return VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV;
#ifdef TIZEN_FEATURE_DSDS
		case STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV2:
			return VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV2;
#endif
		case STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT:
			return VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT;
#ifdef TIZEN_FEATURE_DSDS
		case STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT2:
			return VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT2;
#endif
		case STORAGE_KEY_CELLULAR_PKT_LAST_RCV:
			return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV;
#ifdef TIZEN_FEATURE_DSDS
		case STORAGE_KEY_CELLULAR_PKT_LAST_RCV2:
			return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV2;
#endif
		case STORAGE_KEY_CELLULAR_PKT_LAST_SNT:
			return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT;
#ifdef TIZEN_FEATURE_DSDS
		case STORAGE_KEY_CELLULAR_PKT_LAST_SNT2:
			return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT2;
#endif
		case STORAGE_KEY_LANGUAGE_SET:
			return VCONFKEY_LANGSET;
		case STORAGE_KEY_FLIGHT_MODE_BOOL:
			return VCONFKEY_TELEPHONY_FLIGHT_MODE;
		case STORAGE_KEY_TESTMODE_FAST_DORMANCY:
			return VCONFKEY_TESTMODE_FAST_DORMANCY;
#ifdef TIZEN_FEATURE_DSDS
		case STORAGE_KEY_TESTMODE_FAST_DORMANCY2:
			return VCONFKEY_TESTMODE_FAST_DORMANCY2;
#endif
		case STORAGE_KEY_IDLE_SCREEN_LAUNCHED:
			return VCONFKEY_IDLE_SCREEN_LAUNCHED;
		case STORAGE_KEY_POWER_SAVING_MODE:
			return VCONFKEY_SETAPPL_PSMODE;
		case STORAGE_KEY_SETAPPL_NETWORK_RESTRICT_MODE:
			return VCONFKEY_SETAPPL_NETWORK_RESTRICT_MODE;
		case STORAGE_KEY_SETAPPL_MOBILE_DATA_POPUP_DONE_BOOL:
			return VCONFKEY_SETAPPL_MOBILE_DATA_POPUP_DONE;
		case STORAGE_KEY_MSG_SERVER_READY_BOOL:
			return VCONFKEY_MSG_SERVER_READY;
		case STORAGE_KEY_SETAPPL_STATE_DATA_ROAMING_APP_STATUS:
			return VCONFKEY_SETAPPL_STATE_DATA_ROAMING_APP_STATUS;
		case STORAGE_KEY_TELEPHONY_PREFERRED_VOICE_SUBSCRIPTION:
			return VCONFKEY_TELEPHONY_PREFERRED_VOICE_SUBSCRIPTION;
		case STORAGE_KEY_TELEPHONY_DUALSIM_DEFAULT_SERVICE_INT:
			return VCONFKEY_TELEPHONY_DB_DEFAULT_SUBS;
		case STORAGE_KEY_WIFI_STATE_INT:
			return VCONFKEY_WIFI_STATE;
		case STORAGE_KEY_WECONN_ALL_CONNECTED:
			return VCONFKEY_WECONN_ALL_CONNECTED;
		case STORAGE_KEY_SAP_CONNECTION_TYPE:
			return VCONFKEY_SAP_CONNECTION_TYPE;
		default:
			break;
	}

	return NULL;
}

static enum tcore_storage_key convert_vconf_to_strgkey(const gchar* key)
{
	if (g_str_equal(key, VCONFKEY_TELEPHONY_PLMN) == TRUE) {
		return STORAGE_KEY_TELEPHONY_PLMN;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_LAC) == TRUE) {
		return STORAGE_KEY_TELEPHONY_LAC;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_CELLID) == TRUE) {
		return STORAGE_KEY_TELEPHONY_CELLID;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVCTYPE) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SVCTYPE;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVC_CS) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SVC_CS;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVC_PS) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SVC_PS;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVC_ROAM) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SVC_ROAM;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_PB_INIT) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SIM_PB_INIT;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_CALL_FORWARD_STATE) == TRUE) {
		return STORAGE_KEY_TELEPHONY_CALL_FORWARD_STATE;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_TAPI_STATE) == TRUE) {
		return STORAGE_KEY_TELEPHONY_TAPI_STATE;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SPN_DISP_CONDITION) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SPN_DISP_CONDITION;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_RSSI) == TRUE) {
		return STORAGE_KEY_TELEPHONY_RSSI;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_READY) == TRUE) {
		return STORAGE_KEY_TELEPHONY_READY;
	}
	else if (g_str_equal(key, VCONFKEY_3G_ENABLE) == TRUE) {
		return STORAGE_KEY_3G_ENABLE;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_DB_DEFAULT_DATA_SUBS) == TRUE) {
		return STORAGE_KEY_TELEPHONY_DUALSIM_DEFAULT_DATA_SERVICE_INT;
	}
	else if (g_str_equal(key, VCONFKEY_SETAPPL_STATE_DATA_ROAMING_BOOL) == TRUE) {
		return STORAGE_KEY_SETAPPL_STATE_DATA_ROAMING_BOOL;
	}
	else if (g_str_equal(key, VCONFKEY_PM_STATE) == TRUE) {
		return STORAGE_KEY_PM_STATE;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_SLOT) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SIM_SLOT;
	}
#ifdef TIZEN_FEATURE_DSDS
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_SLOT2) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SIM_SLOT2;
	}
#endif
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_SLOT_COUNT) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SIM_SLOT_COUNT;
	}
	else if (g_str_equal(key, VCONFKEY_DNET_STATE) == TRUE) {
		return STORAGE_KEY_PACKET_SERVICE_STATE;
	}
#ifdef TIZEN_FEATURE_DSDS
	else if (g_str_equal(key, VCONFKEY_DNET_STATE2) == TRUE) {
		return STORAGE_KEY_PACKET_SERVICE_STATE2;
	}
#endif
	else if (g_str_equal(key, VCONFKEY_PACKET_STATE) == TRUE) {
		return STORAGE_KEY_PACKET_INDICATOR_STATE;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_NWNAME) == TRUE) {
		return STORAGE_KEY_TELEPHONY_NWNAME;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SPN_NAME) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SPN_NAME;
	}
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_STATE) == TRUE) {
		return STORAGE_KEY_CELLULAR_STATE;
	}
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV;
	}
#ifdef TIZEN_FEATURE_DSDS
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV2) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV2;
	}
#endif
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT;
	}
#ifdef TIZEN_FEATURE_DSDS
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT2) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT2;
	}
#endif
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_LAST_RCV;
	}
#ifdef TIZEN_FEATURE_DSDS
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV2) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_LAST_RCV2;
	}
#endif
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_LAST_SNT;
	}
#ifdef TIZEN_FEATURE_DSDS
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT2) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_LAST_SNT2;
	}
#endif
	else if (g_str_equal(key, VCONFKEY_LANGSET) == TRUE) {
		return STORAGE_KEY_LANGUAGE_SET;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_FLIGHT_MODE) == TRUE) {
		return STORAGE_KEY_FLIGHT_MODE_BOOL;
	}
	else if (g_str_equal(key, VCONFKEY_TESTMODE_FAST_DORMANCY) == TRUE) {
		return STORAGE_KEY_TESTMODE_FAST_DORMANCY;
	}
#ifdef TIZEN_FEATURE_DSDS
	else if (g_str_equal(key, VCONFKEY_TESTMODE_FAST_DORMANCY2) == TRUE) {
		return STORAGE_KEY_TESTMODE_FAST_DORMANCY2;
	}
#endif
	else if (g_str_equal(key, VCONFKEY_IDLE_SCREEN_LAUNCHED) == TRUE) {
		return STORAGE_KEY_IDLE_SCREEN_LAUNCHED;
	}
	else if (g_str_equal(key, VCONFKEY_SETAPPL_PSMODE) == TRUE) {
		return STORAGE_KEY_POWER_SAVING_MODE;
	}
	else if (g_str_equal(key, VCONFKEY_SETAPPL_NETWORK_RESTRICT_MODE) == TRUE) {
		return STORAGE_KEY_SETAPPL_NETWORK_RESTRICT_MODE;
	}
	else if (g_str_equal(key, VCONFKEY_SETAPPL_MOBILE_DATA_POPUP_DONE) == TRUE) {
		return STORAGE_KEY_SETAPPL_MOBILE_DATA_POPUP_DONE_BOOL;
	}
	else if (g_str_equal(key, VCONFKEY_MSG_SERVER_READY) == TRUE) {
		return STORAGE_KEY_MSG_SERVER_READY_BOOL;
	}
	else if (g_str_equal(key, VCONFKEY_SETAPPL_STATE_DATA_ROAMING_APP_STATUS) == TRUE) {
		return STORAGE_KEY_SETAPPL_STATE_DATA_ROAMING_APP_STATUS;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_PREFERRED_VOICE_SUBSCRIPTION) == TRUE) {
		return STORAGE_KEY_TELEPHONY_PREFERRED_VOICE_SUBSCRIPTION;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_DB_DEFAULT_SUBS) == TRUE) {
		return STORAGE_KEY_TELEPHONY_DUALSIM_DEFAULT_SERVICE_INT;
	}
	else if (g_str_equal(key, VCONFKEY_WIFI_STATE) == TRUE) {
		return STORAGE_KEY_WIFI_STATE_INT;
	}
	else if(g_str_equal(key, VCONFKEY_WECONN_ALL_CONNECTED) == TRUE) {
		return STORAGE_KEY_WECONN_ALL_CONNECTED;
	}
	else if(g_str_equal(key, VCONFKEY_SAP_CONNECTION_TYPE) == TRUE) {
		return STORAGE_KEY_SAP_CONNECTION_TYPE;
	}

	return 0;
}

static void* create_handle(Storage *strg, const char *path)
{
	void *tmp = NULL;
	if(!strg)
		return NULL;

	tmp = malloc(sizeof(char));
	return tmp;
}

static gboolean remove_handle(Storage *strg, void *handle)
{
	if(!strg || !handle)
		return FALSE;

	free(handle);
	return TRUE;
}

static gboolean set_int(Storage *strg, enum tcore_storage_key key, int value)
{
	const gchar *s_key = NULL;

	if (!strg)
		return FALSE;

	if(key & STORAGE_KEY_INT)
		s_key = convert_strgkey_to_vconf(key);

	if(!s_key)
		return FALSE;

	if (vconf_set_int(s_key, value) == 0)
		return TRUE;

	return FALSE;
}

static gboolean set_bool(Storage *strg, enum tcore_storage_key key, gboolean value)
{
	const gchar *s_key = NULL;

	if (!strg)
		return FALSE;

	if(key & STORAGE_KEY_BOOL)
		s_key = convert_strgkey_to_vconf(key);

	if(!s_key)
		return FALSE;

	vconf_set_bool(s_key, value);
	return TRUE;
}

static gboolean set_string(Storage *strg, enum tcore_storage_key key, const char *value)
{
	const gchar *s_key = NULL;

	if (!strg)
		return FALSE;

	if(key & STORAGE_KEY_STRING)
		s_key = convert_strgkey_to_vconf(key);

	if(!s_key)
		return FALSE;

	vconf_set_str(s_key, value);
	return TRUE;
}

static int get_int(Storage *strg, enum tcore_storage_key key)
{
	int value = -1;
	const gchar *s_key = NULL;

	if (!strg)
		return value;

	if(key & STORAGE_KEY_INT)
		s_key = convert_strgkey_to_vconf(key);

	if(s_key != NULL)
		vconf_get_int(s_key, &value);

	return value;
}

static gboolean get_bool(Storage *strg, enum tcore_storage_key key)
{
	gboolean value = FALSE;
	const gchar *s_key = NULL;

	if (!strg)
		return value;

	if(key & STORAGE_KEY_BOOL)
		s_key = convert_strgkey_to_vconf(key);

	if(s_key != NULL)
		vconf_get_bool(s_key, &value);

	return value;
}

static char *get_string(Storage *strg, enum tcore_storage_key key)
{
	const gchar *s_key = NULL;

	if (!strg)
		return NULL;

	if(key & STORAGE_KEY_STRING)
		s_key = convert_strgkey_to_vconf(key);

	if(s_key != NULL)
		return vconf_get_str(s_key);

	return NULL;
}

static void __vconfkey_callback(keynode_t* node, void* data)
{
	int type = 0;
	const char *vkey = NULL;
	GVariant *value = NULL;
	enum tcore_storage_key s_key = 0;
	Storage *strg = NULL;

	strg = (Storage *)data;
	vkey = vconf_keynode_get_name(node);
	type = vconf_keynode_get_type(node);
	s_key = convert_vconf_to_strgkey(vkey);

	if(type == VCONF_TYPE_STRING){
		gchar *tmp;
		tmp = (gchar *)vconf_keynode_get_str(node);
		value = g_variant_new_string( tmp );
	}
	else if(type == VCONF_TYPE_INT){
		gint32 tmp = 0;
		tmp = vconf_keynode_get_int(node);
		value = g_variant_new_int32( tmp );
	}
	else if(type == VCONF_TYPE_DOUBLE){
		gdouble tmp = 0;
		tmp = vconf_keynode_get_dbl(node);
		value = g_variant_new_double( tmp );
	}
	else if(type == VCONF_TYPE_BOOL){
		gboolean tmp = FALSE;
		tmp = vconf_keynode_get_bool(node);
		value = g_variant_new_boolean( tmp );
	}


	if(callback_dispatch != NULL)
		callback_dispatch(strg, s_key, value);

	if(value)
		g_variant_unref(value);

	return;
}

static gboolean set_key_callback(Storage *strg, enum tcore_storage_key key, TcoreStorageDispatchCallback cb)
{
	const gchar *s_key = NULL;

	if (!strg)
		return FALSE;

	s_key = convert_strgkey_to_vconf(key);
	dbg("s_key (%s)", s_key);

	if(s_key == NULL)
		return FALSE;

	if(callback_dispatch == NULL)
		callback_dispatch = cb;

	vconf_notify_key_changed(s_key, __vconfkey_callback, strg);
	return TRUE;
}

static gboolean remove_key_callback(Storage *strg, enum tcore_storage_key key)
{
	const gchar *s_key = NULL;

	if (!strg)
		return FALSE;

	s_key = convert_strgkey_to_vconf(key);
	dbg("s_key (%s)", s_key);

	if(s_key == NULL)
		return FALSE;

	vconf_ignore_key_changed(s_key, __vconfkey_callback);
	return TRUE;
}

struct storage_operations ops = {
	.create_handle = create_handle,
	.remove_handle = remove_handle,
	.set_int = set_int,
	.set_string = set_string,
	.set_bool = set_bool,
	.get_int = get_int,
	.get_string = get_string,
	.get_bool = get_bool,
	.set_key_callback = set_key_callback,
	.remove_key_callback = remove_key_callback,
};

static void _update_vconf_network_name(CoreObject *o)
{
	char *nwname = NULL;
	char *spnname = NULL;
	enum telephony_network_service_type svc_type;
	enum telephony_network_access_technology svc_act;
	enum tcore_network_name_priority network_name_priority;

	tcore_network_get_service_type(o, &svc_type);
	if (svc_type != NETWORK_SERVICE_TYPE_3G) {
		int current = 0;

		vconf_get_int(VCONFKEY_TELEPHONY_PSTYPE, &current);
		if (current != 0) {
			dbg("force hsdpa state off");
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_NONE);
		}
	}

	tcore_network_get_access_technology(o, &svc_act);
	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SVC_ACT, svc_act);

	tcore_network_get_network_name_priority(o, &network_name_priority);
	switch (network_name_priority) {
		case TCORE_NETWORK_NAME_PRIORITY_SPN:
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_SPN);
			break;

		case TCORE_NETWORK_NAME_PRIORITY_NETWORK:
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_PLMN);
			break;

		case TCORE_NETWORK_NAME_PRIORITY_ANY:
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_SPN_PLMN);
			break;

		default:
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_INVALID);
			break;
	}

	switch (svc_type) {
		case NETWORK_SERVICE_TYPE_UNKNOWN:
		case NETWORK_SERVICE_TYPE_NO_SERVICE:
		case NETWORK_SERVICE_TYPE_EMERGENCY:
		case NETWORK_SERVICE_TYPE_SEARCH:
			break;

		default:
			/* spn */
			spnname = tcore_network_get_network_name(o, TCORE_NETWORK_NAME_TYPE_SPN);
			if (spnname) {
				__vconf_check_and_set_str(VCONFKEY_TELEPHONY_SPN_NAME, spnname);
			}

			/* nitz */
			nwname = tcore_network_get_network_name(o, TCORE_NETWORK_NAME_TYPE_FULL);
			if (nwname && strlen(nwname) > 0) {
				dbg("SPN:[%s] FULL:[%s] prio:[%d] act:[%d] svc_type:[%d]",
						spnname?spnname:"", nwname, network_name_priority, svc_act, svc_type);
				__vconf_check_and_set_str(VCONFKEY_TELEPHONY_NWNAME, nwname);
				break;
			}
			else {
				if (nwname)
					free(nwname);
				nwname = tcore_network_get_network_name(o, TCORE_NETWORK_NAME_TYPE_SHORT);
				if (nwname) {
					dbg("SPN:[%s] SHORT:[%s] prio:[%d] act:[%d] svc_type:[%d]",
						spnname?spnname:"", nwname, network_name_priority, svc_act, svc_type);
					__vconf_check_and_set_str(VCONFKEY_TELEPHONY_NWNAME, nwname);
					break;
				}
			}
			dbg("name is not fixed yet. SPN:[%s] prio:[%d] act:[%d] svc_type:[%d]",
				spnname?spnname:"", network_name_priority, svc_act, svc_type);

			break;
	}
	if (spnname)
		free(spnname);

	if (nwname)
		free(nwname);
}

static enum tcore_hook_return on_hook_network_location_cellinfo(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_location_cellinfo *info = data;

	/*
	 * Backward compatibility for Single SIM (Primary Subscription ONLY)
	 *
	 * In case of Dual SIM, ONLY Primary Subscription's notifications would be
	 * processed
	 */
	if (__vconf_check_process_hook_callback(source) == FALSE) {
		dbg("Notification NOT intended for Primary Subscription");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_CELLID, info->cell_id);
	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_LAC, info->lac);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network_icon_info(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_icon_info *info = data;

	/*
	 * Backward compatibility for Single SIM (Primary Subscription ONLY)
	 *
	 * In case of Dual SIM, ONLY Primary Subscription's notifications would be
	 * processed
	 */
	if (__vconf_check_process_hook_callback(source) == FALSE) {
		dbg("Notification NOT intended for Primary Subscription");
		return TCORE_HOOK_RETURN_CONTINUE;
	}
#ifdef TIZEN_FEATURE_CDMA
	if(info->type & NETWORK_ICON_INFO_ROAM_ICON_MODE) {
		__vconf_check_and_set_int(VCONFKEY_TELEPHONY_ROAM_ICON_MODE, info->roam_icon_mode);
	}
#endif
	if(info->type & NETWORK_ICON_INFO_RSSI) {
		__vconf_check_and_set_int(VCONFKEY_TELEPHONY_RSSI, info->rssi);
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network_registration_status(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_registration_status *info = data;
	int status;
	gboolean roaming_allowed;

	warn("vconf set (cs:[%d] ps:[%d] svctype:[%d] roam:[%d])",
		info->cs_domain_status,info->ps_domain_status,info->service_type,info->roaming_status);

	/*
	 * Backward compatibility for Single SIM (Primary Subscription ONLY)
	 *
	 * In case of Dual SIM, ONLY Primary Subscription's notifications would be
	 * processed
	 */
	if (__vconf_check_process_hook_callback(source) == FALSE) {
		dbg("Notification NOT intended for Primary Subscription");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	/* CS */
	if (info->cs_domain_status == NETWORK_SERVICE_DOMAIN_STATUS_FULL)
		status = 2;
	else
		status = 1;

	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SVC_CS, status);

	/* PS */
	if (info->ps_domain_status == NETWORK_SERVICE_DOMAIN_STATUS_FULL)
		status = 2;
	else
		status = 1;

	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SVC_PS, status);

	/* Service type */
	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SVCTYPE, info->service_type);

	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SVC_ROAM, info->roaming_status);

	_update_vconf_network_name(source);

	vconf_get_bool(VCONFKEY_SETAPPL_STATE_DATA_ROAMING_BOOL, &roaming_allowed);
	if(info->service_type > NETWORK_SERVICE_TYPE_SEARCH && !roaming_allowed && info->roaming_status) {
		int pkg_state;
		vconf_get_int(VCONFKEY_DNET_STATE, &pkg_state);
		if(pkg_state > 0) {
			dbg("Mismatch: hide PS icon.");
			__vconf_check_and_set_int(VCONFKEY_DNET_STATE, VCONFKEY_DNET_OFF);
		}
	}
	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network_change(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_change *info = data;

	msg("vconf set (plmn:[%s] lac:[%d])", info->plmn, info->gsm.lac);

	/*
	 * Backward compatibility for Single SIM (Primary Subscription ONLY)
	 *
	 * In case of Dual SIM, ONLY Primary Subscription's notifications would be
	 * processed
	 */
	if (__vconf_check_process_hook_callback(source) == FALSE) {
		dbg("Notification NOT intended for Primary Subscription");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_PLMN, atoi(info->plmn));
	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_LAC, info->gsm.lac);

	_update_vconf_network_name(source);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network_identity(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_identity *info = data;

	msg("vconf set (plmn:[%s])", info->plmn);

	/*
	 * Backward compatibility for Single SIM (Primary Subscription ONLY)
	 *
	 * In case of Dual SIM, ONLY Primary Subscription's notifications would be
	 * processed
	 */
	if (__vconf_check_process_hook_callback(source) == FALSE) {
		dbg("Notification NOT intended for Primary Subscription");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_PLMN, atoi(info->plmn));

	_update_vconf_network_name(source);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network_default_data_subs (Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_default_data_subs *info = data;

	msg("vconf set (default data subs:[%d])", info->default_subs);
	__vconf_check_and_set_int( VCONFKEY_TELEPHONY_DB_DEFAULT_DATA_SUBS, info->default_subs);
	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_sim_init(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_sim_status *sim  = data;
	const char *cp_name;
	guint slot = 0;

	cp_name = tcore_server_get_cp_name_by_plugin(tcore_object_ref_plugin(source));
	dbg("CP name: [%s]", cp_name);

	if (cp_name != NULL) {
		if (g_str_has_suffix(cp_name, "0"))
			slot = 0;
		else if (g_str_has_suffix(cp_name, "1"))
			slot = 1;
		else {
			err("Vconf keys are not supported for this CP name");
			return TCORE_HOOK_RETURN_CONTINUE;
		}
	} else {
		err("CP Name is NULL");
		return TCORE_HOOK_RETURN_CONTINUE;
	}
	warn("vconf set (sim_status = %d), slot - (%d)", sim->sim_status, slot);

	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_SIM_STATUS, sim->sim_status);

	switch (sim->sim_status) {
		case SIM_STATUS_CARD_ERROR:
		case SIM_STATUS_CARD_CRASHED:
			if (slot == 0)
				vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_CARD_ERROR);
#ifdef TIZEN_FEATURE_DSDS
			else
				vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_CARD_ERROR);
#endif
			__vconf_check_and_set_str(VCONFKEY_TELEPHONY_NWNAME, "SIM Error");
			break;

		case SIM_STATUS_CARD_NOT_PRESENT:
		case SIM_STATUS_CARD_REMOVED:
			if (slot == 0)
				vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_NOT_PRESENT);
#ifdef TIZEN_FEATURE_DSDS
			else
				vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_NOT_PRESENT);
#endif
			__vconf_check_and_set_str(VCONFKEY_TELEPHONY_NWNAME, "NO SIM");
			break;

		case SIM_STATUS_INIT_COMPLETED:
		case SIM_STATUS_INITIALIZING:
		case SIM_STATUS_PIN_REQUIRED:
		case SIM_STATUS_PUK_REQUIRED:
		case SIM_STATUS_LOCK_REQUIRED:
		case SIM_STATUS_CARD_BLOCKED:
		case SIM_STATUS_NCK_REQUIRED:
		case SIM_STATUS_NSCK_REQUIRED:
		case SIM_STATUS_SPCK_REQUIRED:
		case SIM_STATUS_CCK_REQUIRED:
			if (slot == 0)
				vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_INSERTED);
#ifdef TIZEN_FEATURE_DSDS
			else
				vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_INSERTED);
#endif
			break;

		case SIM_STATUS_UNKNOWN:
		default:
			if (slot == 0)
				vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_UNKNOWN);
#ifdef TIZEN_FEATURE_DSDS
			else
				vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_UNKNOWN);
#endif
			break;
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_pb_init(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_phonebook_status *pb  = data;

	dbg("vconf set (pb->b_init = %d)", pb->b_init);

	/*
	 * Backward compatibility for Single SIM (Primary Subscription ONLY)
	 *
	 * In case of Dual SIM, ONLY Primary Subscription's notifications would be
	 * processed
	 */
	if (__vconf_check_process_hook_callback(source) == FALSE) {
		dbg("Notification NOT intended for Primary Subscription");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	if (vconf_set_int(VCONFKEY_TELEPHONY_SIM_PB_INIT, pb->b_init) != 0)
		dbg("[FAIL] UPDATE VCONFKEY_TELEPHONY_SIM_PB_INIT");

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_ps_protocol_status(Server *s, CoreObject *source,
		enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	enum telephony_network_service_type svc_type;
	const struct tnoti_ps_protocol_status *noti = data;

	dbg("vconf set (ps status = %d)", noti->status);

	/*
	 * Backward compatibility for Single SIM (Primary Subscription ONLY)
	 *
	 * In case of Dual SIM, ONLY Primary Subscription's notifications would be
	 * processed
	 */
	if (__vconf_check_process_hook_callback(source) == FALSE) {
		dbg("Notification NOT intended for Primary Subscription");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	vconf_get_int(VCONFKEY_TELEPHONY_SVCTYPE, (int *)&svc_type);
	if (svc_type < (enum telephony_network_service_type)VCONFKEY_TELEPHONY_SVCTYPE_2G) {
		dbg("service state is not available");
		__vconf_check_and_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_NONE);
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	switch (noti->status) {
		case TELEPHONY_HSDPA_OFF:
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_NONE);
			break;

		case TELEPHONY_HSDPA_ON:
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_HSDPA);
			break;

		case TELEPHONY_HSUPA_ON:
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_HSUPA);
			break;

		case TELEPHONY_HSPA_ON:
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_HSPA);
			break;
		case TELEPHONY_HSPAP_ON:
			__vconf_check_and_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_HSPAP);
			break;
		default:
			warn("invalid ps status");
			break;
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_modem_flight_mode(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_modem_flight_mode *flight_mode = data;
	struct vconf_plugin_user_data *ud = user_data;

	/*
	 * Backward compatibility for Single SIM (Primary Subscription ONLY)
	 *
	 * In case of Dual SIM, ONLY Primary Subscription's notifications would be
	 * processed
	 */
	if (__vconf_check_process_hook_callback(source) == FALSE) {
		dbg("Notification NOT intended for Primary Subscription");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	warn("vconf set (flight_mode = %d)", flight_mode->enable);

	vconf_set_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, flight_mode->enable);

	if (flight_mode->enable == 1) {
		if (ud->last_modem_power_state == MODEM_STATE_ONLINE && ud->last_flight_mode_state == FALSE) {
			__vconf_write_power_status_log(ud, MODEM_STATE_LOW);
			ud->last_flight_mode_state = TRUE;
		}
	} else {
		if (ud->last_modem_power_state == MODEM_STATE_LOW && ud->last_flight_mode_state == TRUE) {
			__vconf_write_power_status_log(ud, MODEM_STATE_ONLINE);
			ud->last_flight_mode_state = FALSE;
		}
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_modem_power(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_modem_power *power = data;
	struct vconf_plugin_user_data *ud = user_data;

	/*
	 * Backward compatibility for Single SIM (Primary Subscription ONLY)
	 *
	 * In case of Dual SIM, ONLY Primary Subscription's notifications would be
	 * processed
	 */
	if (__vconf_check_process_hook_callback(source) == FALSE) {
		dbg("Notification NOT intended for Primary Subscription");
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	/* TODO: check if tapi ready needs to be reset in case of cp reset
		Keeping current behavior
	*/
	info("modem state : %d", power->state);

	if (power->state == MODEM_STATE_RESUME) {
		info("tapi ready");
		vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_READY);
	} else if (power->state == MODEM_STATE_ERROR) {
		info("cp crash : all network setting will be reset");
		reset_vconf();
	} else if (power->state == MODEM_STATE_LOW) {
		vconf_set_bool(VCOKFKEY_TELEPHONY_MODEM_STATE, FALSE);
		__vconf_write_power_status_log(ud, MODEM_STATE_LOW);
	} else if (power->state == MODEM_STATE_ONLINE) {
		vconf_set_bool(VCOKFKEY_TELEPHONY_MODEM_STATE, TRUE);
		__vconf_write_power_status_log(ud, MODEM_STATE_ONLINE);
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_bootup_complete(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	info("tapi ready");
	vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_READY);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static void reset_vconf()
{
	vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, "");
	vconf_set_int(VCONFKEY_TELEPHONY_PLMN, 0);
	vconf_set_int(VCONFKEY_TELEPHONY_LAC, 0);
	vconf_set_int(VCONFKEY_TELEPHONY_CELLID, 0);
	vconf_set_int(VCONFKEY_TELEPHONY_SVCTYPE, VCONFKEY_TELEPHONY_SVCTYPE_NONE);
	vconf_set_int(VCONFKEY_TELEPHONY_SVC_CS, VCONFKEY_TELEPHONY_SVC_CS_UNKNOWN);
	vconf_set_int(VCONFKEY_TELEPHONY_SVC_PS, VCONFKEY_TELEPHONY_SVC_PS_UNKNOWN);
	vconf_set_int(VCONFKEY_TELEPHONY_SVC_ROAM, VCONFKEY_TELEPHONY_SVC_ROAM_OFF);
#ifdef TIZEN_FEATURE_CDMA
	vconf_set_int(VCONFKEY_TELEPHONY_ROAM_ICON_MODE, VCONFKEY_TELEPHONY_ROAM_ICON_OFF);
#endif
	vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_UNKNOWN);
	vconf_set_int(VCONFKEY_TELEPHONY_SIM_PB_INIT, VCONFKEY_TELEPHONY_SIM_PB_INIT_NONE);
	vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_INVALID);
	vconf_set_str(VCONFKEY_TELEPHONY_SPN_NAME, "");
	vconf_set_int(VCONFKEY_TELEPHONY_RSSI, VCONFKEY_TELEPHONY_RSSI_0);
	vconf_set_int(VCONFKEY_TELEPHONY_SIM_PB_INIT, VCONFKEY_TELEPHONY_SIM_PB_INIT_NONE);
	vconf_set_int(VCONFKEY_TELEPHONY_NITZ_GMT, 0);
	vconf_set_int(VCONFKEY_TELEPHONY_NITZ_EVENT_GMT, 0);
	vconf_set_str(VCONFKEY_TELEPHONY_NITZ_ZONE, "");

#ifdef TIZEN_FEATURE_DSDS
	vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_UNKNOWN);
#endif
}

static gboolean on_load()
{
	dbg("i'm load!");

	return TRUE;
}

static void __telephony_ready_change_cb(keynode_t *node, void *data)
{
	gboolean enable;

	enable = vconf_keynode_get_bool(node);

	if (enable)
		vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_READY);
	else
		vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_NONE);
}

static gboolean on_init(TcorePlugin *p)
{
	Storage *strg;
	Server *s;
	struct vconf_plugin_user_data *ud;

	if (!p)
		return FALSE;

	dbg("i'm init!");

	strg = tcore_storage_new(p, "vconf", &ops);
	ud = calloc(sizeof(struct vconf_plugin_user_data), 1);
	if (tcore_plugin_link_user_data(p, ud) != TCORE_RETURN_SUCCESS)
		return FALSE;

	reset_vconf();

	vconf_set_int(VCONFKEY_TELEPHONY_SVC_ROAM, VCONFKEY_TELEPHONY_SVC_ROAM_OFF);
	vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_NONE);
	vconf_set_bool(VCONFKEY_TELEPHONY_READY, FALSE);

	s = tcore_plugin_ref_server(p);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_LOCATION_CELLINFO, on_hook_network_location_cellinfo, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_ICON_INFO, on_hook_network_icon_info, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_REGISTRATION_STATUS, on_hook_network_registration_status, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_CHANGE, on_hook_network_change, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_IDENTITY, on_hook_network_identity, strg);
	tcore_server_add_notification_hook(s, TNOTI_SIM_STATUS, on_hook_sim_init, strg);
	tcore_server_add_notification_hook(s, TNOTI_PHONEBOOK_STATUS, on_hook_pb_init, strg);
	tcore_server_add_notification_hook(s, TNOTI_PS_PROTOCOL_STATUS, on_hook_ps_protocol_status, strg);
	tcore_server_add_notification_hook(s, TNOTI_MODEM_POWER, on_hook_modem_power, ud);
	tcore_server_add_notification_hook(s, TNOTI_MODEM_BOOTUP, on_hook_bootup_complete, ud);
	tcore_server_add_notification_hook(s, TNOTI_MODEM_FLIGHT_MODE, on_hook_modem_flight_mode, ud);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_DEFAULT_DATA_SUBSCRIPTION, on_hook_network_default_data_subs, strg);
	vconf_notify_key_changed(VCONFKEY_TELEPHONY_READY, __telephony_ready_change_cb, NULL);
	return TRUE;
}

static void on_unload(TcorePlugin *p)
{
	Storage *strg;
	struct vconf_plugin_user_data *ud;

	if (!p)
		return;

	dbg("i'm unload");

	ud = tcore_plugin_ref_user_data(p);
	if(ud) {
		free(ud);
	}

	strg = tcore_server_find_storage(tcore_plugin_ref_server(p), "vconf");
	if (!strg)
		return;

	tcore_storage_free(strg);
}

EXPORT_API struct tcore_plugin_define_desc plugin_define_desc =
{
	.name = "VCONF_STORAGE",
	.priority = TCORE_PLUGIN_PRIORITY_HIGH - 1,
	.version = PLUGIN_VERSION,
	.load = on_load,
	.init = on_init,
	.unload = on_unload
};

