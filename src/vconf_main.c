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

#include <tcore.h>

#include "vconf_main.h"
#include "vconf_core.h"
#include "vconf_handler.h"

/*
 * VCONF Initialization function
 */
gboolean vconf_main_init(TcorePlugin *p)
{
	gboolean ret;

	if (!p)
		return FALSE;

	dbg("Enter");

	/*
	 * Initialize VCONF 'Core'
	 */
	ret = vconf_core_init(p);
	if (ret == FALSE) {
		err("VCONF Core init failed!");
		return FALSE;
	}

	/*
	 * Initialize VCONF 'Handler'
	 */
	ret = vconf_handler_init(p);
	if (ret == FALSE) {
		err("VCONF Handler init failed!");

		vconf_core_deinit(p);
		return FALSE;
	}


	return TRUE;
}

/*
 * VCONF De-initialization function
 */
void vconf_main_deinit(TcorePlugin *p)
{
	if (!p)
		return;

	dbg("Enter");

	/*
	 * De-initialize VCONF 'Handler'
	 */
	vconf_handler_deinit(p);

	/*
	 * De-initialize VCONF 'Core'
	 */
	vconf_core_deinit(p);
}
