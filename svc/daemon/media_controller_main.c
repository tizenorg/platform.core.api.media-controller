/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "media_controller_private.h"
#include "../media_controller_socket.h"
#include "../media_controller_svc.h"

GMainLoop *g_mc_mainloop = NULL;
static int g_mc_timer_id = 0;

#define MC_MAIN_TIMEOUT_SEC_60	600


void __mc_main_create_timer(int timer_id);

gboolean __mc_main_check_connection(gpointer user_data)
{
	int connection_cnt = 0;
	connection_cnt = mc_svc_get_connection_cnt();
	if ((connection_cnt == -1) || (connection_cnt > 0)) {
		mc_error("[No-error] Timer is Called but there is working Process");

		__mc_main_create_timer(g_mc_timer_id);
	} else {
		mc_error("[No-error] Timer is Called. Now Killing mediacontroller process");
		g_mc_timer_id = 0;

		/*Quit Controller Thread*/
		GMainLoop *mc_svc_mainloop = mc_svc_get_main_loop();
		if (mc_svc_mainloop && g_main_is_running(mc_svc_mainloop)) {
			g_main_loop_quit(mc_svc_mainloop);
		}

		g_main_loop_quit(g_mc_mainloop);
	}

	return FALSE;
}

void __mc_main_create_timer(int timer_id)
{
	if (timer_id > 0)
		g_source_destroy(g_main_context_find_source_by_id(g_main_context_get_thread_default(), timer_id));

	GSource *timer_src = g_timeout_source_new_seconds(MC_MAIN_TIMEOUT_SEC_60);
	g_source_set_callback (timer_src, __mc_main_check_connection, NULL, NULL);
	g_mc_timer_id = g_source_attach (timer_src, g_main_context_get_thread_default());
}

int main(int argc, char **argv)
{
	GThread *svc_thread = NULL;
	int fd = -1;

	/*Init main loop*/
	g_mc_mainloop = g_main_loop_new(NULL, FALSE);

	fd = mc_create_socket_activation();
	if (fd < 0) {
		mc_error("Failed to socekt creation");
	}

	/*create each threads*/
	svc_thread  = g_thread_new("mc_svc_thread", (GThreadFunc)mc_svc_thread, NULL);

	/* Create Timer */
	__mc_main_create_timer(g_mc_timer_id);

	mc_debug("*** Media Controller Daemon is running ***");

	g_main_loop_run(g_mc_mainloop);

	g_thread_join(svc_thread);
	g_main_loop_unref(g_mc_mainloop);

	mc_debug("*** Media Controller Daemon is stopped ***");

	return 0;
}
