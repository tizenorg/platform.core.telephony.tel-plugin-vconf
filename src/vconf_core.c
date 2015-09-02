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

#include <glib.h>
#include <vconf.h>

#include <tcore.h>
#include <server.h>
#include <plugin.h>
#include <storage.h>

#include "vconf_main.h"
#include "vconf_core.h"

static TcoreStorageDispatchCallback callback_dispatch;

static const char *__vconf_convert_strgkey_to_vconf(enum tcore_storage_key key)
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
	case STORAGE_KEY_TELEPHONY_SIM_SLOT2:
		return VCONFKEY_TELEPHONY_SIM_SLOT2;
	case STORAGE_KEY_TELEPHONY_SIM_SLOT_COUNT:
		return VCONFKEY_TELEPHONY_SIM_SLOT_COUNT;
	case STORAGE_KEY_PM_STATE:
		return VCONFKEY_PM_STATE;
	case STORAGE_KEY_PACKET_SERVICE_STATE:
		return VCONFKEY_DNET_STATE;
	case STORAGE_KEY_PACKET_SERVICE_STATE2:
		return VCONFKEY_DNET_STATE2;
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
	case STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV2:
		return VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV2;
	case STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT:
		return VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT;
	case STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT2:
		return VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT2;
	case STORAGE_KEY_CELLULAR_PKT_LAST_RCV:
		return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV;
	case STORAGE_KEY_CELLULAR_PKT_LAST_RCV2:
		return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV2;
	case STORAGE_KEY_CELLULAR_PKT_LAST_SNT:
		return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT;
	case STORAGE_KEY_CELLULAR_PKT_LAST_SNT2:
		return VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT2;
	case STORAGE_KEY_LANGUAGE_SET:
		return VCONFKEY_LANGSET;
	case STORAGE_KEY_FLIGHT_MODE_BOOL:
		return VCONFKEY_TELEPHONY_FLIGHT_MODE;
	case STORAGE_KEY_POWER_SAVING_MODE:
		return VCONFKEY_SETAPPL_PSMODE;
	case STORAGE_KEY_SETAPPL_NETWORK_RESTRICT_MODE:
		return VCONFKEY_SETAPPL_NETWORK_RESTRICT_MODE;
	case STORAGE_KEY_MSG_SERVER_READY_BOOL:
		return VCONFKEY_MSG_SERVER_READY;
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
#ifdef PREPAID_SIM_APN_SUPPORT
	case STORAGE_KEY_PDP_LAST_CONNECTED_CONTEXT_BOOL:
		return VCONFKEY_TELEPHONY_PRIVATE_PDP_LAST_CONNECTED_CONTEXT;
	case STORAGE_KEY_PDP_LAST_CONNECTED_CONTEXT_PROFILE_ID:
		return VCONFKEY_TELEPHONY_PRIVATE_PDP_LAST_CONNECTED_CONTEXT_PROFILE_ID;
	case STORAGE_KEY_TELEPHONY_LAST_CONNECTED_CONTEXT_PLMN:
		return VCONFKEY_TELEPHONY_PRIVATE_PDP_LAST_CONNECTED_CONTEXT_PLMN;
#endif
	default:
	break;
	}

	return NULL;
}

static enum tcore_storage_key __vconf_convert_vconf_to_strgkey(const char *key)
{
	if (g_str_equal(key, VCONFKEY_TELEPHONY_PLMN) == TRUE)
		return STORAGE_KEY_TELEPHONY_PLMN;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_LAC) == TRUE)
		return STORAGE_KEY_TELEPHONY_LAC;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_CELLID) == TRUE)
		return STORAGE_KEY_TELEPHONY_CELLID;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVCTYPE) == TRUE)
		return STORAGE_KEY_TELEPHONY_SVCTYPE;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVC_CS) == TRUE)
		return STORAGE_KEY_TELEPHONY_SVC_CS;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVC_PS) == TRUE)
		return STORAGE_KEY_TELEPHONY_SVC_PS;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVC_ROAM) == TRUE)
		return STORAGE_KEY_TELEPHONY_SVC_ROAM;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_PB_INIT) == TRUE)
		return STORAGE_KEY_TELEPHONY_SIM_PB_INIT;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_CALL_FORWARD_STATE) == TRUE)
		return STORAGE_KEY_TELEPHONY_CALL_FORWARD_STATE;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_TAPI_STATE) == TRUE)
		return STORAGE_KEY_TELEPHONY_TAPI_STATE;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SPN_DISP_CONDITION) == TRUE)
		return STORAGE_KEY_TELEPHONY_SPN_DISP_CONDITION;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_RSSI) == TRUE)
		return STORAGE_KEY_TELEPHONY_RSSI;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_READY) == TRUE)
		return STORAGE_KEY_TELEPHONY_READY;
	else if (g_str_equal(key, VCONFKEY_3G_ENABLE) == TRUE)
		return STORAGE_KEY_3G_ENABLE;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_DB_DEFAULT_DATA_SUBS) == TRUE)
		return STORAGE_KEY_TELEPHONY_DUALSIM_DEFAULT_DATA_SERVICE_INT;
	else if (g_str_equal(key, VCONFKEY_SETAPPL_STATE_DATA_ROAMING_BOOL) == TRUE)
		return STORAGE_KEY_SETAPPL_STATE_DATA_ROAMING_BOOL;
	else if (g_str_equal(key, VCONFKEY_PM_STATE) == TRUE)
		return STORAGE_KEY_PM_STATE;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_SLOT) == TRUE)
		return STORAGE_KEY_TELEPHONY_SIM_SLOT;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_SLOT2) == TRUE)
		return STORAGE_KEY_TELEPHONY_SIM_SLOT2;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_SLOT_COUNT) == TRUE)
		return STORAGE_KEY_TELEPHONY_SIM_SLOT_COUNT;
	else if (g_str_equal(key, VCONFKEY_DNET_STATE) == TRUE)
		return STORAGE_KEY_PACKET_SERVICE_STATE;
	else if (g_str_equal(key, VCONFKEY_DNET_STATE2) == TRUE)
		return STORAGE_KEY_PACKET_SERVICE_STATE2;
	else if (g_str_equal(key, VCONFKEY_PACKET_STATE) == TRUE)
		return STORAGE_KEY_PACKET_INDICATOR_STATE;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_NWNAME) == TRUE)
		return STORAGE_KEY_TELEPHONY_NWNAME;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_SPN_NAME) == TRUE)
		return STORAGE_KEY_TELEPHONY_SPN_NAME;
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_STATE) == TRUE)
		return STORAGE_KEY_CELLULAR_STATE;
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV) == TRUE)
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV;
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV2) == TRUE)
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV2;
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT) == TRUE)
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT;
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT2) == TRUE)
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT2;
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV) == TRUE)
		return STORAGE_KEY_CELLULAR_PKT_LAST_RCV;
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV2) == TRUE)
		return STORAGE_KEY_CELLULAR_PKT_LAST_RCV2;
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT) == TRUE)
		return STORAGE_KEY_CELLULAR_PKT_LAST_SNT;
	else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT2) == TRUE)
		return STORAGE_KEY_CELLULAR_PKT_LAST_SNT2;
	else if (g_str_equal(key, VCONFKEY_LANGSET) == TRUE)
		return STORAGE_KEY_LANGUAGE_SET;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_FLIGHT_MODE) == TRUE)
		return STORAGE_KEY_FLIGHT_MODE_BOOL;
	else if (g_str_equal(key, VCONFKEY_SETAPPL_PSMODE) == TRUE)
		return STORAGE_KEY_POWER_SAVING_MODE;
	else if (g_str_equal(key, VCONFKEY_SETAPPL_NETWORK_RESTRICT_MODE) == TRUE)
		return STORAGE_KEY_SETAPPL_NETWORK_RESTRICT_MODE;
	else if (g_str_equal(key, VCONFKEY_MSG_SERVER_READY) == TRUE)
		return STORAGE_KEY_MSG_SERVER_READY_BOOL;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_PREFERRED_VOICE_SUBSCRIPTION) == TRUE)
		return STORAGE_KEY_TELEPHONY_PREFERRED_VOICE_SUBSCRIPTION;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_DB_DEFAULT_SUBS) == TRUE)
		return STORAGE_KEY_TELEPHONY_DUALSIM_DEFAULT_SERVICE_INT;
	else if (g_str_equal(key, VCONFKEY_WIFI_STATE) == TRUE)
		return STORAGE_KEY_WIFI_STATE_INT;
	else if (g_str_equal(key, VCONFKEY_WECONN_ALL_CONNECTED) == TRUE)
		return STORAGE_KEY_WECONN_ALL_CONNECTED;
	else if (g_str_equal(key, VCONFKEY_SAP_CONNECTION_TYPE) == TRUE)
		return STORAGE_KEY_SAP_CONNECTION_TYPE;
#ifdef PREPAID_SIM_APN_SUPPORT
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_PRIVATE_PDP_LAST_CONNECTED_CONTEXT) == TRUE)
		return STORAGE_KEY_PDP_LAST_CONNECTED_CONTEXT_BOOL;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_PRIVATE_PDP_LAST_CONNECTED_CONTEXT_PROFILE_ID) == TRUE)
		return STORAGE_KEY_PDP_LAST_CONNECTED_CONTEXT_PROFILE_ID;
	else if (g_str_equal(key, VCONFKEY_TELEPHONY_PRIVATE_PDP_LAST_CONNECTED_CONTEXT_PLMN) == TRUE)
		return STORAGE_KEY_TELEPHONY_LAST_CONNECTED_CONTEXT_PLMN;
#endif

	return 0;
}

static void __vconf_key_change_callback(keynode_t *node, void *data)
{
	int type = 0;
	char *vkey = NULL;
	GVariant *value = NULL;
	enum tcore_storage_key s_key = 0;
	Storage *strg = NULL;

	strg = (Storage *)data;
	vkey = vconf_keynode_get_name(node);
	type = vconf_keynode_get_type(node);
	if (vkey != NULL)
		s_key = __vconf_convert_vconf_to_strgkey(vkey);

	if (type == VCONF_TYPE_STRING) {
		gchar *tmp;
		tmp = (char *)vconf_keynode_get_str(node);
		value = g_variant_new_string(tmp);
	} else if (type == VCONF_TYPE_INT) {
		gint32 tmp = 0;
		tmp = vconf_keynode_get_int(node);
		value = g_variant_new_int32(tmp);
	} else if (type == VCONF_TYPE_DOUBLE) {
		gdouble tmp = 0;
		tmp = vconf_keynode_get_dbl(node);
		value = g_variant_new_double(tmp);
	} else if (type == VCONF_TYPE_BOOL) {
		gboolean tmp = FALSE;
		tmp = vconf_keynode_get_bool(node);
		value = g_variant_new_boolean(tmp);
	}


	if (callback_dispatch != NULL)
		callback_dispatch(strg, s_key, value);

	if (value)
		g_variant_unref(value);

	return;
}

static void *create_handle(Storage *strg, const char *path)
{
	void *tmp = NULL;
	if (!strg)
		return NULL;

	tmp = malloc(sizeof(char));
	return tmp;
}

static gboolean remove_handle(Storage *strg, void *handle)
{
	if (!strg || !handle)
		return FALSE;

	free(handle);
	return TRUE;
}

static gboolean set_int(Storage *strg, enum tcore_storage_key key, int value)
{
	const gchar *s_key = NULL;

	if (!strg)
		return FALSE;

	if (key & STORAGE_KEY_INT)
		s_key = __vconf_convert_strgkey_to_vconf(key);

	if (!s_key)
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

	if (key & STORAGE_KEY_BOOL)
		s_key = __vconf_convert_strgkey_to_vconf(key);

	if (!s_key)
		return FALSE;

	vconf_set_bool(s_key, value);
	return TRUE;
}

static gboolean set_string(Storage *strg, enum tcore_storage_key key, const char *value)
{
	const gchar *s_key = NULL;

	if (!strg)
		return FALSE;

	if (key & STORAGE_KEY_STRING)
		s_key = __vconf_convert_strgkey_to_vconf(key);

	if (!s_key)
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

	if (key & STORAGE_KEY_INT)
		s_key = __vconf_convert_strgkey_to_vconf(key);

	if (s_key != NULL)
		vconf_get_int(s_key, &value);

	return value;
}

static gboolean get_bool(Storage *strg, enum tcore_storage_key key)
{
	gboolean value = FALSE;
	const gchar *s_key = NULL;

	if (!strg)
		return value;

	if (key & STORAGE_KEY_BOOL)
		s_key = __vconf_convert_strgkey_to_vconf(key);

	if (s_key != NULL)
		vconf_get_bool(s_key, &value);

	return value;
}

static char *get_string(Storage *strg, enum tcore_storage_key key)
{
	const gchar *s_key = NULL;

	if (!strg)
		return NULL;

	if (key & STORAGE_KEY_STRING)
		s_key = __vconf_convert_strgkey_to_vconf(key);

	if (s_key != NULL)
		return vconf_get_str(s_key);

	return NULL;
}

static gboolean set_key_callback(Storage *strg,
	enum tcore_storage_key key, TcoreStorageDispatchCallback cb)
{
	const gchar *s_key = NULL;

	if (!strg)
		return FALSE;

	s_key = __vconf_convert_strgkey_to_vconf(key);
	dbg("s_key (%s)", s_key);

	if (s_key == NULL)
		return FALSE;

	if (callback_dispatch == NULL)
		callback_dispatch = cb;

	vconf_notify_key_changed(s_key,
		__vconf_key_change_callback, strg);

	return TRUE;
}

static gboolean remove_key_callback(Storage *strg,
	enum tcore_storage_key key)
{
	const gchar *s_key = NULL;

	if (!strg)
		return FALSE;

	s_key = __vconf_convert_strgkey_to_vconf(key);
	dbg("s_key (%s)", s_key);

	if (s_key == NULL)
		return FALSE;

	vconf_ignore_key_changed(s_key,
		__vconf_key_change_callback);

	return TRUE;
}

/*
 * VCONF operations
 */
static struct storage_operations vconf_ops = {
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

/*
 * VCONF Initialization function
 */
gboolean vconf_core_init(TcorePlugin *p)
{
	Storage *strg;

	if (!p)
		return FALSE;

	strg = tcore_storage_new(p, "vconf", &vconf_ops);
	if (strg == NULL) {
		err("Storage creation failed!");
		return FALSE;
	}

	return TRUE;
}

/*
 * VCONF Core De-initialization function
 */
void vconf_core_deinit(TcorePlugin *p)
{
	Storage *strg;

	if (!p)
		return;

	dbg("Enter");

	strg = tcore_server_find_storage(tcore_plugin_ref_server(p), "vconf");
	if (!strg)
		return;

	tcore_storage_free(strg);
}
