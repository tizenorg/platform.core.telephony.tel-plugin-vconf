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

#include "vconf_main.h"
#include "vconf_handler.h"

/*
 * Private data
 */
typedef struct {
	gboolean b_get_nck_retry_count;

	enum modem_state last_modem_power_state;
	gboolean last_flight_mode_state;
	long last_modem_state_timestamp;
} VconfPrivData;

static gboolean __vconf_check_process_hook_callback(CoreObject *co)
{
	const char *cp_name;

	cp_name = tcore_server_get_cp_name_by_plugin(tcore_object_ref_plugin(co));
	dbg("CP name: [%s]", cp_name);

	if (cp_name == NULL)
		return FALSE;

	return g_str_has_suffix(cp_name, "0");
}

static void __vconf_check_and_set_int(const char *in_key, const int intval)
{
	int current;
	vconf_get_int(in_key, &current);
	if (current != intval)
		vconf_set_int(in_key, intval);
}

static void __vconf_check_and_set_str(const char *in_key, const char *strval)
{
	char *current = vconf_get_str(in_key);

	if (current) {
		if (strval && strcmp(current, strval))
			vconf_set_str(in_key, strval);

		free(current);
	} else {
		vconf_set_str(in_key, strval);
	}
}

static void __vconf_write_power_status_log(VconfPrivData *ud, enum modem_state state)
{
	struct sysinfo sys_info;

	if (ud == NULL)
		return;

	if (0 != sysinfo(&sys_info))
		err("sysinfo failed.");

	if (state == MODEM_STATE_ONLINE) {
		if (ud->last_modem_power_state == MODEM_STATE_LOW) {
			int count = 0;
			if (0 == vconf_get_int(VCONFKEY_TELEPHONY_PRIVATE_MODEM_ON_COUNT, &count)) {
				count++;
				if (0 != vconf_set_int(VCONFKEY_TELEPHONY_PRIVATE_MODEM_ON_COUNT, count))
					err("vconf_set_int failed.");
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
			if (0 != vconf_get_int(VCONFKEY_TELEPHONY_PRIVATE_MODEM_ON_COUNT, &count))
				err("vconf_get_int failed.");

			msg("[MODEM ON/OFF] MODEM ON => LOW in %d secs. (modem_on_count=[%d] after boot-up(uptime %ld secs))",
					sys_info.uptime - ud->last_modem_state_timestamp, count, sys_info.uptime);
			ud->last_modem_power_state = state;
			ud->last_modem_state_timestamp = sys_info.uptime;
		}
	}
}

static void __vconf_update_network_name(CoreObject *o)
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
		if (spnname)
			__vconf_check_and_set_str(VCONFKEY_TELEPHONY_SPN_NAME, spnname);

		/* nitz */
		nwname = tcore_network_get_network_name(o, TCORE_NETWORK_NAME_TYPE_FULL);
		if (nwname && strlen(nwname) > 0) {
			dbg("SPN:[%s] FULL:[%s] prio:[%d] act:[%d] svc_type:[%d]",
					spnname ? spnname : "", nwname, network_name_priority, svc_act, svc_type);
			__vconf_check_and_set_str(VCONFKEY_TELEPHONY_NWNAME, nwname);
			break;
		} else {
			if (nwname)
				free(nwname);
			nwname = tcore_network_get_network_name(o, TCORE_NETWORK_NAME_TYPE_SHORT);
			if (nwname) {
				dbg("SPN:[%s] SHORT:[%s] prio:[%d] act:[%d] svc_type:[%d]",
					spnname ? spnname : "", nwname, network_name_priority, svc_act, svc_type);
				__vconf_check_and_set_str(VCONFKEY_TELEPHONY_NWNAME, nwname);
				break;
			}
		}
		dbg("name is not fixed yet. SPN:[%s] prio:[%d] act:[%d] svc_type:[%d]",
			spnname ? spnname : "", network_name_priority, svc_act, svc_type);

	break;
	}

	if (spnname)
		free(spnname);

	if (nwname)
		free(nwname);
}

static void __vconf_reset_vconfkeys()
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

	vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_UNKNOWN);
}

static void __vconf_telephony_ready_change_cb(keynode_t *node, void *data)
{
	gboolean enable;

	enable = vconf_keynode_get_bool(node);
	dbg("Telephony State: [%s]", enable ? "Ready" : "NOT ready");

	if (enable)
		vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_READY);
	else
		vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_NONE);
}

static enum tcore_hook_return vconf_on_hook_network_location_cellinfo(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
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

static enum tcore_hook_return vconf_on_hook_network_icon_info(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
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
	if (info->type & NETWORK_ICON_INFO_ROAM_ICON_MODE)
		__vconf_check_and_set_int(VCONFKEY_TELEPHONY_ROAM_ICON_MODE, info->roam_icon_mode);
#endif
	if (info->type & NETWORK_ICON_INFO_RSSI)
		__vconf_check_and_set_int(VCONFKEY_TELEPHONY_RSSI, info->rssi);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return vconf_on_hook_network_registration_status(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_registration_status *info = data;
	int status;
	gboolean roaming_allowed;

	warn("vconf set (cs:[%d] ps:[%d] svctype:[%d] roam:[%d])",
		info->cs_domain_status, info->ps_domain_status, info->service_type, info->roaming_status);

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

	__vconf_update_network_name(source);

	vconf_get_bool(VCONFKEY_SETAPPL_STATE_DATA_ROAMING_BOOL, &roaming_allowed);
	if (info->service_type > NETWORK_SERVICE_TYPE_SEARCH && !roaming_allowed && info->roaming_status) {
		int pkg_state;
		vconf_get_int(VCONFKEY_DNET_STATE, &pkg_state);
		if (pkg_state > 0) {
			dbg("Mismatch: hide PS icon.");
			__vconf_check_and_set_int(VCONFKEY_DNET_STATE, VCONFKEY_DNET_OFF);
		}
	}
	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return vconf_on_hook_network_change(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
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

	__vconf_update_network_name(source);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return vconf_on_hook_network_identity(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
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

	__vconf_update_network_name(source);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return vconf_on_hook_network_default_data_subs(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_network_default_data_subs *info = data;

	msg("vconf set (default data subs:[%d])", info->default_subs);
	__vconf_check_and_set_int(VCONFKEY_TELEPHONY_DB_DEFAULT_DATA_SUBS, info->default_subs);
	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return vconf_on_hook_sim_init(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_sim_status *sim  = data;
	const char *cp_name;
	guint slot = 0;

	cp_name = tcore_server_get_cp_name_by_plugin(tcore_object_ref_plugin(source));
	dbg("CP name: [%s]", cp_name);

	if (cp_name != NULL) {
		if (g_str_has_suffix(cp_name, "0")) {
			slot = 0;
		} else if (g_str_has_suffix(cp_name, "1")) {
			slot = 1;
		} else {
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
		else
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_CARD_ERROR);
		__vconf_check_and_set_str(VCONFKEY_TELEPHONY_NWNAME, "SIM Error");
	break;

	case SIM_STATUS_CARD_NOT_PRESENT:
	case SIM_STATUS_CARD_REMOVED:
		if (slot == 0)
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_NOT_PRESENT);
		else
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_NOT_PRESENT);
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
		else
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_INSERTED);
	break;

	case SIM_STATUS_UNKNOWN:
	default:
		if (slot == 0)
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT, VCONFKEY_TELEPHONY_SIM_UNKNOWN);
		else
			vconf_set_int(VCONFKEY_TELEPHONY_SIM_SLOT2, VCONFKEY_TELEPHONY_SIM_UNKNOWN);
	break;
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return vconf_on_hook_pb_init(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
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

static enum tcore_hook_return vconf_on_hook_ps_protocol_status(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
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

static enum tcore_hook_return vconf_on_hook_modem_flight_mode(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_modem_flight_mode *flight_mode = data;
	VconfPrivData *ud = user_data;

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

static enum tcore_hook_return vconf_on_hook_modem_power(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
{
	const struct tnoti_modem_power *power = data;
	VconfPrivData *ud = user_data;

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

		/*
		 * Reset Telephony VCONF keys
		 */
		__vconf_reset_vconfkeys();
	} else if (power->state == MODEM_STATE_LOW) {
		vconf_set_bool(VCONFKEY_TELEPHONY_PRIVATE_MODEM_STATE, FALSE);
		__vconf_write_power_status_log(ud, MODEM_STATE_LOW);
	} else if (power->state == MODEM_STATE_ONLINE) {
		vconf_set_bool(VCONFKEY_TELEPHONY_PRIVATE_MODEM_STATE, TRUE);
		__vconf_write_power_status_log(ud, MODEM_STATE_ONLINE);
	}

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return vconf_on_hook_bootup_complete(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
{
	info("tapi ready");
	vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_READY);

	return TCORE_HOOK_RETURN_CONTINUE;
}

static enum tcore_hook_return vconf_on_hook_modem_plugin_removed(Server *s,
	CoreObject *source, enum tcore_notification_command command,
	unsigned int data_len, void *data, void *user_data)
{
	dbg("vconf (Modem Plugin Removed!!!)");

	/*
	 * Reset Telephony VCONF keys
	 */
	__vconf_reset_vconfkeys();

	return TCORE_HOOK_RETURN_CONTINUE;
}

/*
 * Add notification hooks
 */
static void __vconf_register_hooks(Server *s,
	Storage *strg, VconfPrivData *ud)
{
	/* Network related */
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_LOCATION_CELLINFO,
		vconf_on_hook_network_location_cellinfo, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_ICON_INFO,
		vconf_on_hook_network_icon_info, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_REGISTRATION_STATUS,
		vconf_on_hook_network_registration_status, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_CHANGE,
		vconf_on_hook_network_change, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_IDENTITY,
		vconf_on_hook_network_identity, strg);
	tcore_server_add_notification_hook(s, TNOTI_NETWORK_DEFAULT_DATA_SUBSCRIPTION,
		vconf_on_hook_network_default_data_subs, strg);

	/* SIM related */
	tcore_server_add_notification_hook(s, TNOTI_SIM_STATUS,
		vconf_on_hook_sim_init, strg);

	/* Phonebook related */
	tcore_server_add_notification_hook(s, TNOTI_PHONEBOOK_STATUS,
		vconf_on_hook_pb_init, strg);

	/* PS related */
	tcore_server_add_notification_hook(s, TNOTI_PS_PROTOCOL_STATUS,
		vconf_on_hook_ps_protocol_status, strg);

	/* Modem related */
	tcore_server_add_notification_hook(s, TNOTI_MODEM_POWER,
		vconf_on_hook_modem_power, ud);
	tcore_server_add_notification_hook(s, TNOTI_MODEM_BOOTUP,
		vconf_on_hook_bootup_complete, ud);
	tcore_server_add_notification_hook(s, TNOTI_MODEM_FLIGHT_MODE,
		vconf_on_hook_modem_flight_mode, ud);

	/* Plug-in related */
	tcore_server_add_notification_hook(s, TNOTI_SERVER_REMOVED_MODEM_PLUGIN,
		vconf_on_hook_modem_plugin_removed, strg);

	/*
	 * Track Telephony state change
	 *
	 * Need to update VCONFKEY_TELEPHONY_TAPI_STATE,
	 * key based on Telephony state change
	 */
	vconf_notify_key_changed(VCONFKEY_TELEPHONY_READY,
		__vconf_telephony_ready_change_cb, NULL);
}

/*
 * Remove notification hooks
 */
static void __vconf_deregister_hooks(Server *s)
{
	/* Network related */
	tcore_server_remove_notification_hook(s, vconf_on_hook_network_location_cellinfo);
	tcore_server_remove_notification_hook(s, vconf_on_hook_network_icon_info);
	tcore_server_remove_notification_hook(s, vconf_on_hook_network_registration_status);
	tcore_server_remove_notification_hook(s, vconf_on_hook_network_change);
	tcore_server_remove_notification_hook(s, vconf_on_hook_network_identity);
	tcore_server_remove_notification_hook(s, vconf_on_hook_network_default_data_subs);

	/* SIM related */
	tcore_server_remove_notification_hook(s, vconf_on_hook_sim_init);

	/* Phonebook related */
	tcore_server_remove_notification_hook(s, vconf_on_hook_pb_init);

	/* PS related */
	tcore_server_remove_notification_hook(s, vconf_on_hook_ps_protocol_status);

	/* Modem related */
	tcore_server_remove_notification_hook(s, vconf_on_hook_modem_power);
	tcore_server_remove_notification_hook(s, vconf_on_hook_bootup_complete);
	tcore_server_remove_notification_hook(s, vconf_on_hook_modem_flight_mode);

	/* Plug-in related */
	tcore_server_remove_notification_hook(s, vconf_on_hook_modem_plugin_removed);

	/*
	 * Track Telephony state change
	 *
	 * Need to update VCONFKEY_TELEPHONY_TAPI_STATE,
	 * key based on Telephony state change
	 */
	vconf_ignore_key_changed(VCONFKEY_TELEPHONY_READY,
		__vconf_telephony_ready_change_cb);

}

/*
 * VCONF Handler Initialization function
 */
gboolean vconf_handler_init(TcorePlugin *p)
{
	Storage *strg;
	Server *s;
	VconfPrivData *ud;

	if (!p)
		return FALSE;

	dbg("Enter");

	s = tcore_plugin_ref_server(p);
	strg = tcore_server_find_storage(s, "vconf");
	if (!strg)
		return FALSE;

	ud = calloc(sizeof(VconfPrivData), 1);
	if (ud == NULL) {
		err("Failed to allocate memory for user_data");
		tcore_storage_free(strg);
		return FALSE;
	}

	if (tcore_plugin_link_user_data(p, ud) != TCORE_RETURN_SUCCESS) {
		err("Failed to link user_data");
		tcore_storage_free(strg);
		free(ud);
		return FALSE;
	}

	/*
	 * Reset Telephony VCONF keys
	 */
	__vconf_reset_vconfkeys();

	/* Reset Telephony states */
	vconf_set_int(VCONFKEY_TELEPHONY_TAPI_STATE, VCONFKEY_TELEPHONY_TAPI_STATE_NONE);
	vconf_set_bool(VCONFKEY_TELEPHONY_READY, FALSE);

	/*
	 * Register for all hook callbacks
	 */
	__vconf_register_hooks(s, strg, ud);

	return TRUE;
}

/*
 * VCONF Handler De-initialization function
 */
void vconf_handler_deinit(TcorePlugin *p)
{
	Server *s;
	VconfPrivData *ud;

	if (!p)
		return;

	dbg("Enter");

	s = tcore_plugin_ref_server(p);

	/*
	 * De-register for all hook callbacks
	 */
	__vconf_deregister_hooks(s);

	ud = tcore_plugin_ref_user_data(p);
	if (ud)
		free(ud);
}
