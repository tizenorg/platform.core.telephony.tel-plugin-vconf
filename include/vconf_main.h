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

#ifndef __VCONF_MAIN_H__
#define __VCONF_MAIN_H__

__BEGIN_DECLS

#include <glib.h>
#include <tcore.h>

/*
 * TODO -
 *Keys need to be moved to vconf-internal-keys package
 */
#define VCONFKEY_WECONN_ALL_CONNECTED	"memory/private/weconn/all_connected" /* True/False */
#define VCONFKEY_SAP_CONNECTION_TYPE	"memory/private/sap/conn_type" /* SAPInterface.h */

gboolean vconf_main_init(TcorePlugin *plugin);
void vconf_main_deinit(TcorePlugin *plugin);

__END_DECLS

#endif /* __VCONF_MAIN_H__ */
