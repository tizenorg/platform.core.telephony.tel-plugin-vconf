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
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

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

static void reset_vconf();

static TcoreStorageDispatchCallback callback_dispatch;

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
		case STORAGE_KEY_TELEPHONY_CALL_STATE:
			return VCONFKEY_TELEPHONY_CALL_STATE;
		case STORAGE_KEY_TELEPHONY_CALL_FORWARD_STATE:
			return VCONFKEY_TELEPHONY_CALL_FORWARD_STATE;
		case STORAGE_KEY_TELEPHONY_TAPI_STATE:
			return VCONFKEY_TELEPHONY_TAPI_STATE;
		case STORAGE_KEY_TELEPHONY_SPN_DISP_CONDITION:
			return VCONFKEY_TELEPHONY_SPN_DISP_CONDITION;
		case STORAGE_KEY_TELEPHONY_ZONE_ZUHAUSE:
			return VCONFKEY_TELEPHONY_ZONE_ZUHAUSE;
		case STORAGE_KEY_TELEPHONY_RSSI:
			return VCONFKEY_TELEPHONY_RSSI;
		case STORAGE_KEY_TELEPHONY_READY:
			return VCONFKEY_TELEPHONY_READY;
		case STORAGE_KEY_TELEPHONY_SIM_SLOT:
			return VCONFKEY_TELEPHONY_SIM_SLOT;
		case STORAGE_KEY_PM_STATE:
			return VCONFKEY_PM_STATE;
		case STORAGE_KEY_PACKET_SERVICE_STATE:
			return VCONFKEY_DNET_STATE;
		case STORAGE_KEY_MESSAGE_NETWORK_MODE:
			return VCONFKEY_MESSAGE_NETWORK_MODE;
		case STORAGE_KEY_3G_ENABLE:
			return VCONFKEY_3G_ENABLE;
		case STORAGE_KEY_SETAPPL_STATE_DATA_ROAMING_BOOL:
			return VCONFKEY_SETAPPL_STATE_DATA_ROAMING_BOOL;
		case STORAGE_KEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL:
			return VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL;
		case STORAGE_KEY_TELEPHONY_NWNAME:
			return VCONFKEY_TELEPHONY_NWNAME;
		case STORAGE_KEY_TELEPHONY_SPN_NAME:
			return VCONFKEY_TELEPHONY_SPN_NAME;
		case STORAGE_KEY_TELEPHONY_IMEI:
			return VCONFKEY_TELEPHONY_IMEI;
		case STORAGE_KEY_TELEPHONY_SUBSCRIBER_NUMBER:
			return VCONFKEY_TELEPHONY_SUBSCRIBER_NUMBER;
		case STORAGE_KEY_TELEPHONY_SWVERSION:
			return VCONFKEY_TELEPHONY_SWVERSION;
		case STORAGE_KEY_TELEPHONY_HWVERSION:
			return VCONFKEY_TELEPHONY_HWVERSION;
		case STORAGE_KEY_TELEPHONY_IMEI_FACTORY_REBOOT:
			return VCONFKEY_TELEPHONY_IMEI_FACTORY_REBOOT;
		case STORAGE_KEY_TELEPHONY_SIM_FACTORY_MODE:
			return VCONFKEY_TELEPHONY_SIM_FACTORY_MODE;
		case STORAGE_KEY_TELEPHONY_FACTORY_KSTRINGB:
			return VCONFKEY_TELEPHONY_FACTORY_KSTRINGB;
		case STORAGE_KEY_TELEPHONY_IMSI:
			return "db/private/tel-plugin-vconf/imsi";
		case STORAGE_KEY_CELLULAR_STATE:
			return VCONFKEY_NETWORK_CELLULAR_STATE;
		case STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV:
			return VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV;
		case STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT:
			return VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT;
		case STORAGE_KEY_CELLULAR_PKT_LAST_RCV:
			return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV;
		case STORAGE_KEY_CELLULAR_PKT_LAST_SNT:
			return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT;
		case STORAGE_KEY_LANGUAGE_SET:
			return VCONFKEY_LANGSET;
		case STORAGE_KEY_FLIGHT_MODE_BOOL:
			return VCONFKEY_TELEPHONY_FLIGHT_MODE;
		case STORAGE_KEY_IDLE_SCREEN_LAUNCHED_BOOL:
			return VCONFKEY_IDLE_SCREEN_LAUNCHED;
		case STORAGE_KEY_CISSAPPL_SHOW_MY_NUMBER_INT:
			return VCONFKEY_CISSAPPL_SHOW_MY_NUMBER_INT;
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
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_CALL_STATE) == TRUE) {
		return STORAGE_KEY_TELEPHONY_CALL_STATE;
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
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_ZONE_ZUHAUSE) == TRUE) {
		return STORAGE_KEY_TELEPHONY_ZONE_ZUHAUSE;
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
	else if (g_str_equal(key, VCONFKEY_SETAPPL_STATE_DATA_ROAMING_BOOL) == TRUE) {
		return STORAGE_KEY_SETAPPL_STATE_DATA_ROAMING_BOOL;
	}
	else if (g_str_equal(key, VCONFKEY_PM_STATE) == TRUE) {
		return STORAGE_KEY_PM_STATE;
	}
	else if (g_str_equal(key, VCONFKEY_MESSAGE_NETWORK_MODE) == TRUE) {
		return STORAGE_KEY_MESSAGE_NETWORK_MODE;
	}
	else if (g_str_equal(key, VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL) == TRUE) {
		return STORAGE_KEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_SLOT) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SIM_SLOT;
	}
	else if (g_str_equal(key, VCONFKEY_DNET_STATE) == TRUE) {
		return STORAGE_KEY_PACKET_SERVICE_STATE;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_NWNAME) == TRUE) {
		return STORAGE_KEY_TELEPHONY_NWNAME;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SPN_NAME) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SPN_NAME;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_IMEI) == TRUE) {
		return STORAGE_KEY_TELEPHONY_IMEI;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SUBSCRIBER_NUMBER) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SUBSCRIBER_NUMBER;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SWVERSION) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SWVERSION;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_HWVERSION) == TRUE) {
		return STORAGE_KEY_TELEPHONY_HWVERSION;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_IMEI_FACTORY_REBOOT) == TRUE) {
		return STORAGE_KEY_TELEPHONY_IMEI_FACTORY_REBOOT;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_FACTORY_MODE) == TRUE) {
		return STORAGE_KEY_TELEPHONY_SIM_FACTORY_MODE;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_FACTORY_KSTRINGB) == TRUE) {
		return STORAGE_KEY_TELEPHONY_FACTORY_KSTRINGB;
	}
	else if (g_str_equal(key, "db/private/tel-plugin-vconf/imsi") == TRUE) {
		return STORAGE_KEY_TELEPHONY_IMSI;
	}
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_STATE) == TRUE) {
		return STORAGE_KEY_CELLULAR_STATE;
	}
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV;
	}
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT;
	}
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_LAST_RCV;
	}
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_LAST_SNT;
	}
	else if (g_str_equal(key, VCONFKEY_LANGSET) == TRUE) {
		return STORAGE_KEY_LANGUAGE_SET;
	}
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_FLIGHT_MODE) == TRUE) {
		return STORAGE_KEY_FLIGHT_MODE_BOOL;
	}
	else if (g_str_equal(key, VCONFKEY_IDLE_SCREEN_LAUNCHED) == TRUE) {
		return STORAGE_KEY_IDLE_SCREEN_LAUNCHED_BOOL;
	}
	else if (g_str_equal(key, VCONFKEY_CISSAPPL_SHOW_MY_NUMBER_INT) == TRUE) {
		return STORAGE_KEY_CISSAPPL_SHOW_MY_NUMBER_INT;
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

	vconf_set_int(s_key, value);
	return TRUE;
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
	char *vkey = NULL;
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

static void _update_vconf_network_name(CoreObject *o, const char *plmn)
{
	struct tcore_network_operator_info *noi = NULL;
	char *tmp;
	enum telephony_network_service_type svc_type;
	enum telephony_network_access_technology svc_act;
	enum tcore_network_name_priority network_name_priority;
	char mcc[4] = { 0, };
	char mnc[4] = { 0, };
	char *plmn_str = NULL;

	if (plmn)
		plmn_str = (char *)plmn;
	else
		plmn_str = tcore_network_get_plmn(o);

	if (plmn_str) {
		snprintf(mcc, 4, "%s", plmn_str);
		snprintf(mnc, 4, "%s", plmn_str+3);

		if (mnc[2] == '#')
			mnc[2] = '\0';
	}

	tcore_network_get_service_type(o, &svc_type);
	if (svc_type != NETWORK_SERVICE_TYPE_3G) {
		int current = 0;

		vconf_get_int(VCONFKEY_TELEPHONY_PSTYPE, &current);
		if (current != 0) {
			dbg("force hsdpa state off");
			vconf_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_NONE);
		}
	}

	tcore_network_get_access_technology(o, &svc_act);
	vconf_set_int(VCONFKEY_TELEPHONY_SVC_ACT, svc_act);

	tcore_network_get_network_name_priority(o, &network_name_priority);
	switch (network_name_priority) {
		case TCORE_NETWORK_NAME_PRIORITY_SPN:
			vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_SPN);
			break;

		case TCORE_NETWORK_NAME_PRIORITY_NETWORK:
			vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_PLMN);
			break;

		case TCORE_NETWORK_NAME_PRIORITY_ANY:
			vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_SPN_PLMN);
			break;

		default:
			vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_INVALID);
			break;
	}

	switch (svc_type) {
		case NETWORK_SERVICE_TYPE_2G:
		case NETWORK_SERVICE_TYPE_2_5G:
		case NETWORK_SERVICE_TYPE_2_5G_EDGE:
		case NETWORK_SERVICE_TYPE_3G:
		case NETWORK_SERVICE_TYPE_HSDPA:
			/* spn */
			tmp = tcore_network_get_network_name(o, TCORE_NETWORK_NAME_TYPE_SPN);
			if (tmp) {
				dbg("SPN[%s]", tmp);
				vconf_set_str(VCONFKEY_TELEPHONY_SPN_NAME, tmp);
				free(tmp);
			}

			/* nitz */
			tmp = tcore_network_get_network_name(o, TCORE_NETWORK_NAME_TYPE_FULL);
			if (tmp && strlen(tmp) > 0) {
				dbg("NWNAME = NITZ_FULL[%s]", tmp);
				vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, tmp);
				free(tmp);
				break;
			}
			else {
				tmp = tcore_network_get_network_name(o, TCORE_NETWORK_NAME_TYPE_SHORT);
				if (tmp) {
					dbg("NWNAME = NITZ_SHORT[%s]", tmp);
					vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, tmp);
					free(tmp);
					break;
				}
			}

			/* pre-define table */
			noi = tcore_network_operator_info_find(o, mcc, mnc);
			if (noi) {
				dbg("%s-%s: country=[%s], oper=[%s]", mcc, mnc, noi->country, noi->name);
				dbg("NWNAME = pre-define table[%s]", noi->name);
				vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, noi->name);
			}
			else {
				dbg("%s-%s: no network operator name", mcc, mnc);
				vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, plmn_str);
			}
			break;

		default:
			break;
	}

	if (!plmn)
		free(plmn_str);
}

static enum tcore_hook_return on_hook_network_location_cellinfo(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_location_cellinfo *info = data;

	dbg("vconf set");

	vconf_set_int(VCONFKEY_TELEPHONY_CELLID, info->cell_id);
	vconf_set_int(VCONFKEY_TELEPHONY_LAC, info->lac);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network_icon_info(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_icon_info *info = data;

	dbg("vconf set (rssi=%d)", info->rssi);

	vconf_set_int(VCONFKEY_TELEPHONY_RSSI, info->rssi);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network_registration_status(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_registration_status *info = data;
	int current;
	int status;

	dbg("vconf set");

	/* CS */
	if (info->cs_domain_status == NETWORK_SERVICE_DOMAIN_STATUS_FULL)
		status = 2;
	else
		status = 1;

	vconf_get_int(VCONFKEY_TELEPHONY_SVC_CS, &current);
	if (current != status)
		vconf_set_int(VCONFKEY_TELEPHONY_SVC_CS, status);

	/* PS */
	if (info->ps_domain_status == NETWORK_SERVICE_DOMAIN_STATUS_FULL)
		status = 2;
	else
		status = 1;

	vconf_get_int(VCONFKEY_TELEPHONY_SVC_PS, &current);
	if (current != status)
		vconf_set_int(VCONFKEY_TELEPHONY_SVC_PS, status);

	/* Service type */
	vconf_get_int(VCONFKEY_TELEPHONY_SVCTYPE, &current);
	if (current != (int) info->service_type)
		vconf_set_int(VCONFKEY_TELEPHONY_SVCTYPE, info->service_type);

	switch(info->service_type) {
		case NETWORK_SERVICE_TYPE_UNKNOWN:
		case NETWORK_SERVICE_TYPE_NO_SERVICE:
			vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, "No Service");
			break;

		case NETWORK_SERVICE_TYPE_EMERGENCY:
			vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, "EMERGENCY");
			break;

		case NETWORK_SERVICE_TYPE_SEARCH:
			vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, "Searching...");
			break;
		default:
			break;
	}

	vconf_set_int(VCONFKEY_TELEPHONY_SVC_ROAM, info->roaming_status);

	_update_vconf_network_name(source, NULL);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_network_change(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_change *info = data;

	dbg("vconf set");

	vconf_set_int(VCONFKEY_TELEPHONY_PLMN, atoi(info->plmn));
	vconf_set_int(VCONFKEY_TELEPHONY_LAC, info->gsm.lac);

	_update_vconf_network_name(source, info->plmn);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_sim_init(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_sim_status *sim  = data;

	dbg("vconf set (sim_status = %d)", sim->sim_status);

	switch (sim->sim_status) {
		case SIM_STATUS_CARD_ERROR:
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_CARD_ERROR);
			vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, "SIM Error");
			break;

		case SIM_STATUS_CARD_NOT_PRESENT:
		case SIM_STATUS_CARD_REMOVED:
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_NOT_PRESENT);
			vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, "NO SIM");
			break;

		case SIM_STATUS_INIT_COMPLETED:
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_INSERTED);
			break;

		case SIM_STATUS_INITIALIZING:
		case SIM_STATUS_PIN_REQUIRED:
		case SIM_STATUS_PUK_REQUIRED:
		case SIM_STATUS_LOCK_REQUIRED:
		case SIM_STATUS_CARD_BLOCKED:
		case SIM_STATUS_NCK_REQUIRED:
		case SIM_STATUS_NSCK_REQUIRED:
		case SIM_STATUS_SPCK_REQUIRED:
		case SIM_STATUS_CCK_REQUIRED:
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_INSERTED);
			break;

		default:
			break;
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_pb_init(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_phonebook_status *pb  = data;

	dbg("vconf set (pb->b_init = %d)", pb->b_init);

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

	vconf_get_int(VCONFKEY_TELEPHONY_SVCTYPE, (int *)&svc_type);
	if (svc_type < (enum telephony_network_service_type)VCONFKEY_TELEPHONY_SVCTYPE_2G) {
		dbg("service state is not available");
		vconf_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_NONE);
		return TCORE_HOOK_RETURN_CONTINUE;
	}

	switch (noti->status) {
		case TELEPHONY_HSDPA_OFF:
			vconf_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_NONE);
			break;

		case TELEPHONY_HSDPA_ON:
			vconf_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_HSDPA);
			break;

		case TELEPHONY_HSUPA_ON:
			vconf_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_HSUPA);
			break;

		case TELEPHONY_HSPA_ON:
			vconf_set_int(VCONFKEY_TELEPHONY_PSTYPE, VCONFKEY_TELEPHONY_PSTYPE_HSPA);
			break;
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return on_hook_modem_power(Server *s, CoreObject *source, enum tcore_notification_command command, unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_modem_power *power = data;

	dbg("vconf set (power->state = %d)", power->state);

	if (power->state == MODEM_STATE_ONLINE) {
		dbg("tapi ready");
		vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_READY);
	} else if (power->state == MODEM_STATE_ERROR) {

		dbg("cp crash : all network setting will be reset");
		reset_vconf();

	} else {
		dbg("tapi none");
		vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_NONE);
	}

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
	vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_UNKNOWN);
	vconf_set_int(VCONFKEY_TELEPHONY_SIM_PB_INIT, VCONFKEY_TELEPHONY_SIM_PB_INIT_NONE);
	vconf_set_int(VCONFKEY_TELEPHONY_CALL_STATE, VCONFKEY_TELEPHONY_CALL_CONNECT_IDLE);
	vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_NONE);
	vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_INVALID);
	vconf_set_str(VCONFKEY_TELEPHONY_SPN_NAME, "");
	vconf_set_int(VCONFKEY_TELEPHONY_ZONE_ZUHAUSE, 0);
	vconf_set_int(VCONFKEY_TELEPHONY_RSSI, VCONFKEY_TELEPHONY_RSSI_0);
	vconf_set_str(VCONFKEY_TELEPHONY_IMEI, "deprecated_vconf_imei");
	vconf_set_str(VCONFKEY_TELEPHONY_SUBSCRIBER_NUMBER, "");
	vconf_set_int(VCONFKEY_TELEPHONY_SIM_PB_INIT, VCONFKEY_TELEPHONY_SIM_PB_INIT_NONE);
	vconf_set_bool(VCONFKEY_TELEPHONY_READY, 0);
	vconf_set_int(VCONFKEY_TELEPHONY_NITZ_GMT, 0);
	vconf_set_int(VCONFKEY_TELEPHONY_NITZ_EVENT_GMT, 0);
	vconf_set_str(VCONFKEY_TELEPHONY_NITZ_ZONE, "");
}

static gboolean on_load()
{
	dbg("i'm load!");

	return TRUE;
}

static gboolean on_init(TcorePlugin *p)
{
	Storage *strg;
	Server *s;

	if (!p)
		return FALSE;

	dbg("i'm init!");

	strg = tcore_storage_new(p, "vconf", &ops);

	reset_vconf();

	vconf_set_int(VCONFKEY_TELEPHONY_SVC_ROAM, VCONFKEY_TELEPHONY_SVC_ROAM_OFF);

	s = tcore_plugin_ref_server(p);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_LOCATION_CELLINFO, on_hook_network_location_cellinfo, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_ICON_INFO, on_hook_network_icon_info, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_REGISTRATION_STATUS, on_hook_network_registration_status, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_CHANGE, on_hook_network_change, strg);
	tcore_server_add_notification_hook(s, TNOTI_SIM_STATUS, on_hook_sim_init, strg);
	tcore_server_add_notification_hook(s, TNOTI_PHONEBOOK_STATUS, on_hook_pb_init, strg);
	tcore_server_add_notification_hook(s, TNOTI_PS_PROTOCOL_STATUS, on_hook_ps_protocol_status, strg);
	tcore_server_add_notification_hook(s, TNOTI_MODEM_POWER, on_hook_modem_power, strg);

	return TRUE;
}

static void on_unload(TcorePlugin *p)
{
	Storage *strg;

	if (!p)
		return;

	dbg("i'm unload");

	strg = tcore_server_find_storage(tcore_plugin_ref_server(p), "vconf");
	if (!strg)
		return;

	tcore_storage_free(strg);
}

struct tcore_plugin_define_desc plugin_define_desc =
{
	.name = "VCONF_STORAGE",
	.priority = TCORE_PLUGIN_PRIORITY_HIGH - 1,
	.version = PLUGIN_VERSION,
	.load = on_load,
	.init = on_init,
	.unload = on_unload
};
