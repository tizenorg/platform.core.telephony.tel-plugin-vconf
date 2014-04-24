/*
 * tel-plugin-vconf
 *
 * Copyright (c) 2013 Samsung Electronics Co. Ltd. All rights reserved.
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

#include <stdlib.h>
#include <glib.h>
#include <vconf.h>

#include <tcore.h>
#include <server.h>
#include <plugin.h>
#include <storage.h>
#include <co_network.h>
#include <co_sim.h>

#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION 1
#endif

static void reset_vconf();

static TcoreStorageDispatchCallback callback_dispatch;

static const gchar *convert_strgkey_to_vconf(TcoreStorageKey key)
{
	switch (key) {
	case STORAGE_KEY_PLMN:
		return VCONFKEY_TELEPHONY_PLMN;
	case STORAGE_KEY_LAC:
		return VCONFKEY_TELEPHONY_LAC;
	case STORAGE_KEY_CELLID:
		return VCONFKEY_TELEPHONY_CELLID;
	case STORAGE_KEY_SVCTYPE:
		return VCONFKEY_TELEPHONY_SVCTYPE;
	case STORAGE_KEY_SVC_CS:
		return VCONFKEY_TELEPHONY_SVC_CS;
	case STORAGE_KEY_SVC_PS:
		return VCONFKEY_TELEPHONY_SVC_PS;
	case STORAGE_KEY_SVC_ROAM:
		return VCONFKEY_TELEPHONY_SVC_ROAM;
	case STORAGE_KEY_SIM_PB_INIT:
		return VCONFKEY_TELEPHONY_SIM_PB_INIT;
	case STORAGE_KEY_CALL_FORWARD_STATE:
		return VCONFKEY_TELEPHONY_CALL_FORWARD_STATE;
	case STORAGE_KEY_TAPI_STATE:
		return VCONFKEY_TELEPHONY_TAPI_STATE;
	case STORAGE_KEY_SPN_DISP_CONDITION:
		return VCONFKEY_TELEPHONY_SPN_DISP_CONDITION;
	case STORAGE_KEY_RSSI:
		return VCONFKEY_TELEPHONY_RSSI;
	case STORAGE_KEY_READY:
		return VCONFKEY_TELEPHONY_READY;
	case STORAGE_KEY_SIM_SLOT:
		return VCONFKEY_TELEPHONY_SIM_SLOT;
	case STORAGE_KEY_PM_STATE:
		return VCONFKEY_PM_STATE;
	case STORAGE_KEY_PACKET_SERVICE_STATE:
		return VCONFKEY_DNET_STATE;
	case STORAGE_KEY_PACKET_INDICATOR_STATE:
		return VCONFKEY_PACKET_STATE;
	case STORAGE_KEY_DATA_ENABLE:
		return VCONFKEY_3G_ENABLE;
	case STORAGE_KEY_SETAPPL_STATE_DATA_ROAMING:
		return VCONFKEY_SETAPPL_STATE_DATA_ROAMING_BOOL;
	case STORAGE_KEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE:
		return VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL;
	case STORAGE_KEY_NWNAME:
		return VCONFKEY_TELEPHONY_NWNAME;
	case STORAGE_KEY_SPN_NAME:
		return VCONFKEY_TELEPHONY_SPN_NAME;
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
	case STORAGE_KEY_FLIGHT_MODE:
		return VCONFKEY_TELEPHONY_FLIGHT_MODE;
	case STORAGE_KEY_IDLE_SCREEN_LAUNCHED:
		return VCONFKEY_IDLE_SCREEN_LAUNCHED;
	default:
		break;
	}

	err("Unknown storage key");
	return NULL;
}

static TcoreStorageKey convert_vconf_to_strgkey(const gchar *key)
{
	if (g_str_equal(key, VCONFKEY_TELEPHONY_PLMN) == TRUE) {
		return STORAGE_KEY_PLMN;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_LAC) == TRUE) {
		return STORAGE_KEY_LAC;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_CELLID) == TRUE) {
		return STORAGE_KEY_CELLID;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVCTYPE) == TRUE) {
		return STORAGE_KEY_SVCTYPE;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVC_CS) == TRUE) {
		return STORAGE_KEY_SVC_CS;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVC_PS) == TRUE) {
		return STORAGE_KEY_SVC_PS;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_SVC_ROAM) == TRUE) {
		return STORAGE_KEY_SVC_ROAM;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_PB_INIT) == TRUE) {
		return STORAGE_KEY_SIM_PB_INIT;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_CALL_FORWARD_STATE) == TRUE) {
		return STORAGE_KEY_CALL_FORWARD_STATE;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_TAPI_STATE) == TRUE) {
		return STORAGE_KEY_TAPI_STATE;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_SPN_DISP_CONDITION) == TRUE) {
		return STORAGE_KEY_SPN_DISP_CONDITION;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_RSSI) == TRUE) {
		return STORAGE_KEY_RSSI;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_READY) == TRUE) {
		return STORAGE_KEY_READY;
	} else if (g_str_equal(key, VCONFKEY_3G_ENABLE) == TRUE) {
		return STORAGE_KEY_DATA_ENABLE;
	} else if (g_str_equal(key, VCONFKEY_SETAPPL_STATE_DATA_ROAMING_BOOL) == TRUE) {
		return STORAGE_KEY_SETAPPL_STATE_DATA_ROAMING;
	} else if (g_str_equal(key, VCONFKEY_PM_STATE) == TRUE) {
		return STORAGE_KEY_PM_STATE;
	} else if (g_str_equal(key, VCONFKEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE_BOOL) == TRUE) {
		return STORAGE_KEY_SETAPPL_STATE_AUTOMATIC_TIME_UPDATE;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_SIM_SLOT) == TRUE) {
		return STORAGE_KEY_SIM_SLOT;
	} else if (g_str_equal(key, VCONFKEY_DNET_STATE) == TRUE) {
		return STORAGE_KEY_PACKET_SERVICE_STATE;
	} else if (g_str_equal(key, VCONFKEY_PACKET_STATE) == TRUE) {
		return STORAGE_KEY_PACKET_INDICATOR_STATE;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_NWNAME) == TRUE) {
		return STORAGE_KEY_NWNAME;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_SPN_NAME) == TRUE) {
		return STORAGE_KEY_SPN_NAME;
	} else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_STATE) == TRUE) {
		return STORAGE_KEY_CELLULAR_STATE;
	} else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_RCV) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_RCV;
	} else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_TOTAL_SNT) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_TOTAL_SNT;
	} else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_RCV) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_LAST_RCV;
	} else if (g_str_equal(key, VCONFKEY_NETWORK_CELLULAR_PKT_LAST_SNT) == TRUE) {
		return STORAGE_KEY_CELLULAR_PKT_LAST_SNT;
	} else if (g_str_equal(key, VCONFKEY_LANGSET) == TRUE) {
		return STORAGE_KEY_LANGUAGE_SET;
	} else if (g_str_equal(key, VCONFKEY_TELEPHONY_FLIGHT_MODE) == TRUE) {
		return STORAGE_KEY_FLIGHT_MODE;
	} else if (g_str_equal(key, VCONFKEY_IDLE_SCREEN_LAUNCHED) == TRUE) {
		return STORAGE_KEY_IDLE_SCREEN_LAUNCHED;
	}

	err("Unknown vconf key");
	return 0;
}

static gboolean set_int(TcoreStorage *strg, TcoreStorageKey key, gint value)
{
	gint ret = -1;
	const gchar *s_key = NULL;
	tcore_check_return_value_assert(strg != NULL, FALSE);

	s_key = convert_strgkey_to_vconf(key);
	tcore_check_return_value_assert(s_key != NULL, FALSE);

	ret = vconf_set_int(s_key, value);
	tcore_check_return_value(ret == 0, FALSE);
	return TRUE;
}

static gboolean set_bool(TcoreStorage *strg, TcoreStorageKey key, gboolean value)
{
	gint ret = -1;
	const gchar *s_key = NULL;
	tcore_check_return_value_assert(strg != NULL, FALSE);

	s_key = convert_strgkey_to_vconf(key);
	tcore_check_return_value_assert(s_key != NULL, FALSE);

	ret = vconf_set_bool(s_key, value);
	tcore_check_return_value(ret == 0, FALSE);
	return TRUE;
}

static gboolean set_string(TcoreStorage *strg, TcoreStorageKey key, const gchar *value)
{
	gint ret = -1;
	const gchar *s_key = NULL;
	tcore_check_return_value_assert(strg != NULL, FALSE);

	s_key = convert_strgkey_to_vconf(key);
	tcore_check_return_value_assert(s_key != NULL, FALSE);

	ret = vconf_set_str(s_key, value);
	tcore_check_return_value(ret == 0, FALSE);
	return TRUE;
}

static gint get_int(TcoreStorage *strg, TcoreStorageKey key)
{
	gint ret = -1;
	gint value = -1;
	const gchar *s_key = NULL;
	tcore_check_return_value_assert(strg != NULL, -1);

	s_key = convert_strgkey_to_vconf(key);
	tcore_check_return_value_assert(s_key != NULL, -1);

	ret = vconf_get_int(s_key, &value);
	tcore_check_return_value(ret == 0, -1);
	return value;
}

static gboolean get_bool(TcoreStorage *strg, TcoreStorageKey key)
{
	gint ret = -1;
	gboolean value = FALSE;
	const gchar *s_key = NULL;
	tcore_check_return_value_assert(strg != NULL, FALSE);

	s_key = convert_strgkey_to_vconf(key);
	tcore_check_return_value_assert(s_key != NULL, FALSE);

	ret = vconf_get_bool(s_key, &value);
	tcore_check_return_value(ret == 0, FALSE);
	return value;
}

static gchar *get_string(TcoreStorage *strg, TcoreStorageKey key)
{
	gchar *value = NULL;
	const gchar *s_key = NULL;
	tcore_check_return_value_assert(strg != NULL, NULL);

	s_key = convert_strgkey_to_vconf(key);
	tcore_check_return_value_assert(s_key != NULL, NULL);

	value = vconf_get_str(s_key);
	return value;
}

static void __vconfkey_callback(keynode_t *node, void *data)
{
	gint type = 0;
	const gchar *vkey = NULL;
	GVariant *value = NULL;
	TcoreStorageKey s_key = 0;
	TcoreStorage *strg = NULL;

	strg = (TcoreStorage *)data;
	vkey = vconf_keynode_get_name(node);
	type = vconf_keynode_get_type(node);
	s_key = convert_vconf_to_strgkey(vkey);
	tcore_check_return_assert(s_key != 0);

	if (type == VCONF_TYPE_STRING) {
		gchar *tmp;
		tmp = (gchar *)vconf_keynode_get_str(node);
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
	} else {
		err("Not supported type");
	}

	if (callback_dispatch != NULL)
		callback_dispatch(strg, s_key, value);
}

static gboolean set_key_callback(TcoreStorage *strg, TcoreStorageKey key,
		TcoreStorageDispatchCallback cb)
{
	const gchar *s_key = NULL;
	tcore_check_return_value_assert(strg != NULL, FALSE);

	s_key = convert_strgkey_to_vconf(key);
	tcore_check_return_value_assert(s_key != NULL, FALSE);

	dbg("s_key (%s)", s_key);

	if (callback_dispatch == NULL)
		callback_dispatch = cb;

	vconf_notify_key_changed(s_key, __vconfkey_callback, strg);
	return TRUE;
}

static gboolean remove_key_callback(TcoreStorage *strg, TcoreStorageKey key)
{
	const gchar *s_key = NULL;
	tcore_check_return_value_assert(strg != NULL, FALSE);

	s_key = convert_strgkey_to_vconf(key);
	tcore_check_return_value_assert(s_key != NULL, FALSE);

	dbg("s_key (%s)", s_key);

	vconf_ignore_key_changed(s_key, __vconfkey_callback);
	return TRUE;
}

static TcoreStorageOperations ops = {
	.set_int = set_int,
	.set_string = set_string,
	.set_bool = set_bool,
	.get_int = get_int,
	.get_string = get_string,
	.get_bool = get_bool,
	.set_key_callback = set_key_callback,
	.remove_key_callback = remove_key_callback,
};

static void __vconf_set_service_type(const TelNetworkRegStatusInfo *reg_status)
{
	guint type_key = VCONFKEY_TELEPHONY_SVCTYPE_NONE;

	switch (reg_status->act) {
	case TEL_NETWORK_ACT_UNKNOWN:
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_NONE;
		break;
	case TEL_NETWORK_ACT_GSM:
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_2G;
		break;
	case TEL_NETWORK_ACT_GPRS:
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_2_5G;
		break;
	case TEL_NETWORK_ACT_EGPRS:
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_2_5G_EDGE;
		break;
	case TEL_NETWORK_ACT_UMTS:
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_3G;
		break;
	case TEL_NETWORK_ACT_HSDPA:
	case TEL_NETWORK_ACT_HSUPA:
	case TEL_NETWORK_ACT_HSPA:
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_HSDPA;
		break;
	case TEL_NETWORK_ACT_LTE:
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_LTE;
		break;
	default:
		err("Unknown Access Technology");
		break;
	}

	if (reg_status->cs_status == TEL_NETWORK_REG_STATUS_REGISTERED
			|| reg_status->cs_status == TEL_NETWORK_REG_STATUS_ROAMING
			|| reg_status->ps_status == TEL_NETWORK_REG_STATUS_REGISTERED
			|| reg_status->ps_status == TEL_NETWORK_REG_STATUS_ROAMING) {
		/* No Change */
	} else if (reg_status->cs_status == TEL_NETWORK_REG_STATUS_UNREGISTERED
			&& reg_status->ps_status == TEL_NETWORK_REG_STATUS_UNREGISTERED) {
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_NOSVC;
	} else if (reg_status->cs_status == TEL_NETWORK_REG_STATUS_SEARCHING
			|| reg_status->ps_status == TEL_NETWORK_REG_STATUS_SEARCHING) {
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_SEARCH;
	} else if (reg_status->cs_status == TEL_NETWORK_REG_STATUS_DENIED
			|| reg_status->ps_status == TEL_NETWORK_REG_STATUS_DENIED) {
		type_key = VCONFKEY_TELEPHONY_SVCTYPE_EMERGENCY;
	}

	vconf_set_int(VCONFKEY_TELEPHONY_SVCTYPE, type_key);
}

static void __vconf_set_service_act(const TelNetworkAct act)
{
	guint act_key = VCONFKEY_TELEPHONY_SVC_ACT_NONE;

	switch (act) {
	case TEL_NETWORK_ACT_UNKNOWN:
		act_key = VCONFKEY_TELEPHONY_SVC_ACT_NONE;
		break;
	case TEL_NETWORK_ACT_GSM:
		act_key = VCONFKEY_TELEPHONY_SVC_ACT_GSM;
		break;
	case TEL_NETWORK_ACT_GPRS:
		act_key = VCONFKEY_TELEPHONY_SVC_ACT_GPRS;
		break;
	case TEL_NETWORK_ACT_EGPRS:
		act_key = VCONFKEY_TELEPHONY_SVC_ACT_EGPRS;
		break;
	case TEL_NETWORK_ACT_UMTS:
		act_key = VCONFKEY_TELEPHONY_SVC_ACT_UMTS;
		break;
	case TEL_NETWORK_ACT_HSDPA:
	case TEL_NETWORK_ACT_HSUPA:
	case TEL_NETWORK_ACT_HSPA:
		/* SVC ACT VCONF does not have HSPA. so, set to UMTS */
		act_key = VCONFKEY_TELEPHONY_SVC_ACT_UMTS;
		break;
	case TEL_NETWORK_ACT_LTE:
		act_key = VCONFKEY_TELEPHONY_SVC_ACT_LTE;
		break;
	default:
		err("Unknown Access Technology");
		break;
	}

	vconf_set_int(VCONFKEY_TELEPHONY_SVC_ACT, act_key);
}

static void __vconf_set_spn_and_disp_condition(TcorePlugin *plugin)
{
	CoreObject *co_sim;
	TelSimSpnDispCondition disp_condition;
	gchar *sim_spn = NULL;

	co_sim = tcore_plugin_ref_core_object(plugin, CORE_OBJECT_TYPE_SIM);
	tcore_sim_get_disp_condition(co_sim, &disp_condition);

	/* Set VCONFKEY_TELEPHONY_SPN_DISP_CONDITION */
	switch (disp_condition) {
	case TEL_SIM_DISP_SPN:
		vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION,
			VCONFKEY_TELEPHONY_DISP_SPN);
		break;
	case TEL_SIM_DISP_PLMN:
		vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION,
			VCONFKEY_TELEPHONY_DISP_PLMN);
		break;
	case TEL_SIM_DISP_SPN_PLMN:
		vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION,
			VCONFKEY_TELEPHONY_DISP_SPN_PLMN);
		break;
	case TEL_SIM_DISP_INVALID:
		vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION,
			VCONFKEY_TELEPHONY_DISP_INVALID);
		break;
	}

	/* Set VCONFKEY_TELEPHONY_SPN_NAME */
	tcore_sim_get_spn(co_sim, &sim_spn);
	if (sim_spn != NULL) {
		dbg("Service Provider Name: [%s]", sim_spn);
		vconf_set_str(VCONFKEY_TELEPHONY_SPN_NAME, sim_spn);
		tcore_free(sim_spn);
	}
}

static TcoreHookReturn on_hook_network_location_cellinfo(TcorePlugin *source,
		TcoreNotification command, guint data_len, void *data,
		void *user_data)
{
	const TelNetworkCellInfo *cell_info = data;
	tcore_check_return_value_assert(cell_info != NULL,
		TCORE_HOOK_RETURN_STOP_PROPAGATION);

	dbg("Entry, cell_id: [%d], lac: [%d]",
		cell_info->cell_id, cell_info->lac);

	vconf_set_int(VCONFKEY_TELEPHONY_CELLID, cell_info->cell_id);
	vconf_set_int(VCONFKEY_TELEPHONY_LAC, cell_info->lac);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static TcoreHookReturn on_hook_network_rssi(TcorePlugin *source,
		TcoreNotification command, guint data_len, void *data,
		void *user_data)
{
	const guint rssi = *(guint *)data;

	dbg("Entry, rssi: [%d]", rssi);

	vconf_set_int(VCONFKEY_TELEPHONY_RSSI, rssi);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static TcoreHookReturn on_hook_network_registration_status(TcorePlugin *source,
		TcoreNotification command, guint data_len, void *data,
		void *user_data)
{
	const TelNetworkRegStatusInfo *reg_status = data;
	guint cs_key = VCONFKEY_TELEPHONY_SVC_CS_OFF;
	guint ps_key = VCONFKEY_TELEPHONY_SVC_PS_OFF;
	gboolean roaming_status = FALSE;
	gchar *network_name = NULL;
	guint ps_type = VCONFKEY_TELEPHONY_PSTYPE_NONE;
	tcore_check_return_value_assert(reg_status != NULL,
		TCORE_HOOK_RETURN_STOP_PROPAGATION);

	dbg("Entry, cs_status: [%d], ps_status: [%d], act: [%d]",
		reg_status->cs_status, reg_status->ps_status, reg_status->act);

	/* Set VCONFKEY_TELEPHONY_SVC_CS, VCONFKEY_TELEPHONY_SVC_ROAM */
	if (reg_status->cs_status == TEL_NETWORK_REG_STATUS_REGISTERED) {
		dbg("Circuit Switched ON");
		cs_key = VCONFKEY_TELEPHONY_SVC_CS_ON;
	} else if (reg_status->cs_status == TEL_NETWORK_REG_STATUS_ROAMING) {
		dbg("Circuit Switched ON, Roaming ON");
		cs_key = VCONFKEY_TELEPHONY_SVC_CS_ON;
		roaming_status = TRUE;
	}
	vconf_set_int(VCONFKEY_TELEPHONY_SVC_CS, cs_key);
	vconf_set_int(VCONFKEY_TELEPHONY_SVC_ROAM, roaming_status);

	/* Set VCONFKEY_TELEPHONY_SVC_PS */
	if (reg_status->ps_status == TEL_NETWORK_REG_STATUS_REGISTERED
			|| reg_status->ps_status == TEL_NETWORK_REG_STATUS_ROAMING) {
		dbg("Packet Switched ON");
		ps_key = VCONFKEY_TELEPHONY_SVC_PS_ON;
	}
	vconf_set_int(VCONFKEY_TELEPHONY_SVC_PS, ps_key);

	/* Set VCONFKEY_TELEPHONY_SVCTYPE */
	__vconf_set_service_type(reg_status);

	/* Set VCONFKEY_TELEPHONY_SVC_ACT */
	__vconf_set_service_act(reg_status->act);

	/* Set VCONFKEY_TELEPHONY_NWNAME in case of NOT registered */
	switch (reg_status->cs_status) {
	case TEL_NETWORK_REG_STATUS_UNKNOWN:
		network_name = g_strdup("Unknown");
		break;
	case TEL_NETWORK_REG_STATUS_UNREGISTERED:
		network_name = g_strdup("No Service");
		break;
	case TEL_NETWORK_REG_STATUS_SEARCHING:
		network_name = g_strdup("Searching...");
		break;
	case TEL_NETWORK_REG_STATUS_DENIED:
		network_name = g_strdup("EMERGENCY");
		break;
	case TEL_NETWORK_REG_STATUS_REGISTERED:
	case TEL_NETWORK_REG_STATUS_ROAMING:
		break;
	}
	if (network_name != NULL) {
		vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, network_name);
		tcore_free(network_name);
	}

	/* Set VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_SPN_NAME */
	__vconf_set_spn_and_disp_condition(source);

	/* Set VCONFKEY_TELEPHONY_PSTYPE */
	if (reg_status->ps_status == TEL_NETWORK_REG_STATUS_REGISTERED
			|| reg_status->ps_status == TEL_NETWORK_REG_STATUS_ROAMING) {
		switch (reg_status->act) {
		case TEL_NETWORK_ACT_HSDPA:
			ps_type = VCONFKEY_TELEPHONY_PSTYPE_HSDPA;
			break;
		case TEL_NETWORK_ACT_HSUPA:
			ps_type = VCONFKEY_TELEPHONY_PSTYPE_HSUPA;
			break;
		case TEL_NETWORK_ACT_HSPA:
			ps_type = VCONFKEY_TELEPHONY_PSTYPE_HSPA;
			break;
		default:
			/* ps_type is NONE */
			break;
		}
	}
	vconf_set_int(VCONFKEY_TELEPHONY_PSTYPE, ps_type);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static TcoreHookReturn on_hook_network_identity(TcorePlugin *source,
		TcoreNotification command, guint data_len, void *data,
		void *user_data)
{
	const TelNetworkIdentityInfo *net_identity = data;
	gchar *network_name = NULL;
	tcore_check_return_value_assert(net_identity != NULL,
		TCORE_HOOK_RETURN_STOP_PROPAGATION);

	dbg("Entry, plmn: [%s], short_name: [%s], long_name: [%s]",
		net_identity->plmn, net_identity->short_name, net_identity->long_name);

	/* Set VCONFKEY_TELEPHONY_PLMN */
	if (net_identity->plmn && strlen(net_identity->plmn) > 0)
		vconf_set_int(VCONFKEY_TELEPHONY_PLMN, atoi(net_identity->plmn));
	else
		err("No PLMN");

	/* Set VCONFKEY_TELEPHONY_NWNAME
	 * Network name priority: long name > short name > name from db table > plmn
	 */
	if (net_identity->long_name && strlen(net_identity->long_name) > 0) {
		network_name = g_strdup(net_identity->long_name);
	} else if (net_identity->short_name && strlen(net_identity->short_name) > 0) {
		network_name = g_strdup(net_identity->short_name);
	} else if (net_identity->plmn && strlen(net_identity->plmn) > 0) {
		CoreObject *co_network;
		gchar *operator_name = NULL;
		co_network = tcore_plugin_ref_core_object(source, CORE_OBJECT_TYPE_NETWORK);
		tcore_network_get_operator_name(co_network, net_identity->plmn, &operator_name);
		if (operator_name != NULL) {
			dbg("PLMN: [%s], Operator Name: [%s]", net_identity->plmn, operator_name);
			network_name = g_strdup(operator_name);
			tcore_free(operator_name);
		} else {
			dbg("No Operator Name(PLMN: [%s])", net_identity->plmn);
			network_name = g_strdup(net_identity->plmn);
		}
	} else {
		err("No Notification Data");
	}

	/* Set VCONFKEY_TELEPHONY_NWNAME */
	if (network_name != NULL) {
		vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, network_name);
		tcore_free(network_name);
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static TcoreHookReturn on_hook_sim_init(TcorePlugin *source,
		TcoreNotification command, guint data_len, void *data,
		void *user_data)
{
	const TelSimCardStatusInfo *sim_status_info = data;
	guint sim_slot = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
	gchar *network_name = NULL;
	tcore_check_return_value_assert(sim_status_info != NULL,
		TCORE_HOOK_RETURN_STOP_PROPAGATION);

	dbg("Entry, sim_status: [%d]", sim_status_info->status);

	/* Set VCONFKEY_TELEPHONY_SIM_SLOT */
	switch (sim_status_info->status) {
	case TEL_SIM_STATUS_UNKNOWN:
		sim_slot = VCONFKEY_TELEPHONY_SIM_UNKNOWN;
		break;
	case TEL_SIM_STATUS_CARD_ERROR:
		sim_slot = VCONFKEY_TELEPHONY_SIM_CARD_ERROR;
		network_name = g_strdup("SIM Error");
		break;
	case TEL_SIM_STATUS_CARD_NOT_PRESENT:
	case TEL_SIM_STATUS_CARD_REMOVED:
		sim_slot = VCONFKEY_TELEPHONY_SIM_NOT_PRESENT;
		network_name = g_strdup("NO SIM");
		break;
	case TEL_SIM_STATUS_SIM_INIT_COMPLETED:
	case TEL_SIM_STATUS_SIM_CARD_POWEROFF:
	case TEL_SIM_STATUS_SIM_INITIALIZING:
	case TEL_SIM_STATUS_SIM_PIN_REQUIRED:
	case TEL_SIM_STATUS_SIM_PUK_REQUIRED:
	case TEL_SIM_STATUS_SIM_LOCK_REQUIRED:
	case TEL_SIM_STATUS_CARD_BLOCKED:
	case TEL_SIM_STATUS_SIM_NCK_REQUIRED:
	case TEL_SIM_STATUS_SIM_NSCK_REQUIRED:
	case TEL_SIM_STATUS_SIM_SPCK_REQUIRED:
	case TEL_SIM_STATUS_SIM_CCK_REQUIRED:
		sim_slot = VCONFKEY_TELEPHONY_SIM_INSERTED;
		break;
	}
	vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, sim_slot);

	/* Set VCONFKEY_TELEPHONY_NWNAME in case of SIM NOT inserted */
	if (network_name != NULL) {
		vconf_set_str(VCONFKEY_TELEPHONY_NWNAME, network_name);
		tcore_free(network_name);
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static TcoreHookReturn on_hook_pb_init(TcorePlugin *source,
		TcoreNotification command, guint data_len, void *data,
		void *user_data)
{
	const TelPbInitInfo *init_info = data;

	dbg("Entry, pb_status: [%s]", init_info->init_status ? "INIT COMPLETED" : "NONE");

	/* Set VCONFKEY_TELEPHONY_SIM_PB_INIT */
	vconf_set_int(VCONFKEY_TELEPHONY_SIM_PB_INIT, init_info->init_status);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static TcoreHookReturn on_hook_modem_power(TcorePlugin *source,
		TcoreNotification command, guint data_len, void *data,
		void *user_data)
{
	const TelModemPowerStatus power_status = *(TelModemPowerStatus *)data;

	dbg("Entry, power_status: [%d]", power_status);

	/* Set VCONFKEY_TELEPHONY_TAPI_STATE */
	switch (power_status) {
	case TEL_MODEM_POWER_ON:
		dbg("TAPI Ready");
		vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE,
			VCONFKEY_TELEPHONY_TAPI_STATE_READY);
		break;
	case TEL_MODEM_POWER_ERROR:
		dbg("CP Crash, All Network setting will be reset");
		reset_vconf();
		break;
	case TEL_MODEM_POWER_OFF:
		dbg("TAPI None");
		vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE,
			VCONFKEY_TELEPHONY_TAPI_STATE_NONE);
		break;
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static gboolean __vconf_add_notification_hook(TcorePlugin *modem_plugin)
{
	tcore_check_return_value_assert(modem_plugin != NULL, FALSE);

	tcore_plugin_add_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_NETWORK_LOCATION_CELLINFO,
		on_hook_network_location_cellinfo, NULL);
	tcore_plugin_add_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_NETWORK_RSSI,
		on_hook_network_rssi, NULL);
	tcore_plugin_add_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_NETWORK_REGISTRATION_STATUS,
		on_hook_network_registration_status, NULL);
	tcore_plugin_add_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_NETWORK_IDENTITY,
		on_hook_network_identity, NULL);
	tcore_plugin_add_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_SIM_STATUS,
		on_hook_sim_init, NULL);
	tcore_plugin_add_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_PHONEBOOK_STATUS,
		on_hook_pb_init, NULL);
	tcore_plugin_add_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_MODEM_POWER,
		on_hook_modem_power, NULL);

	return TRUE;
}

static TcoreHookReturn on_hook_modem_plugin_added(Server *server,
		TcoreServerNotification command, guint data_len, void *data,
		void *user_data)
{
	__vconf_add_notification_hook((TcorePlugin *)data);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static TcoreHookReturn on_hook_modem_plugin_removed(Server *server,
		TcoreServerNotification command, guint data_len, void *data,
		void *user_data)
{
	TcorePlugin *modem_plugin = data;

	tcore_plugin_remove_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_NETWORK_LOCATION_CELLINFO,
		on_hook_network_location_cellinfo);
	tcore_plugin_remove_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_NETWORK_RSSI,
		on_hook_network_rssi);
	tcore_plugin_remove_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_NETWORK_REGISTRATION_STATUS,
		on_hook_network_registration_status);
	tcore_plugin_remove_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_NETWORK_IDENTITY,
		on_hook_network_identity);
	tcore_plugin_remove_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_SIM_STATUS,
		on_hook_sim_init);
	tcore_plugin_remove_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_PHONEBOOK_STATUS,
		on_hook_pb_init);
	tcore_plugin_remove_notification_hook(modem_plugin,
		TCORE_NOTIFICATION_MODEM_POWER,
		on_hook_modem_power);

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
	vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_NONE);
	vconf_set_int(VCONFKEY_TELEPHONY_SPN_DISP_CONDITION, VCONFKEY_TELEPHONY_DISP_INVALID);
	vconf_set_str(VCONFKEY_TELEPHONY_SPN_NAME, "");
	vconf_set_int(VCONFKEY_TELEPHONY_RSSI, VCONFKEY_TELEPHONY_RSSI_0);
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

static gboolean on_init(TcorePlugin *plugin)
{
	Server *server;
	GSList *list;
	tcore_check_return_value_assert(plugin != NULL, FALSE);

	dbg("i'm init!");

	tcore_storage_new(plugin, "vconf", &ops);

	reset_vconf();

	server = tcore_plugin_ref_server(plugin);
	list = tcore_server_get_modem_plugin_list(server);
	while (list) {
		TcorePlugin *modem_plugin = list->data;
		dbg("Register for pre-loaded Modem Plug-ins");
		__vconf_add_notification_hook(modem_plugin);
		list = g_slist_next(list);
	}
	g_slist_free(list);

	dbg("Register for post-loaded Modem Plug-ins");
	tcore_server_add_notification_hook(server,
		TCORE_SERVER_NOTIFICATION_ADDED_MODEM_PLUGIN,
		on_hook_modem_plugin_added, NULL);

	tcore_server_add_notification_hook(server,
		TCORE_SERVER_NOTIFICATION_REMOVED_MODEM_PLUGIN,
		on_hook_modem_plugin_removed, NULL);

	return TRUE;
}

static void on_unload(TcorePlugin *plugin)
{
	Server *server;
	TcoreStorage *strg;
	tcore_check_return_assert(plugin != NULL);

	dbg("i'm unload");

	server = tcore_plugin_ref_server(plugin);
	tcore_server_remove_notification_hook(server, on_hook_modem_plugin_added);
	tcore_server_remove_notification_hook(server, on_hook_modem_plugin_removed);

	strg = tcore_server_find_storage(server, "vconf");
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
