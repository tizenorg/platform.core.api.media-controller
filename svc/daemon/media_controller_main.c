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

GMainLoop *g_mainloop = NULL;

int main(int argc, char **argv)
{
	GThread *svc_thread = NULL;

	/*Init main loop*/
	g_mainloop = g_main_loop_new(NULL, FALSE);

	/*create each threads*/
	svc_thread  = g_thread_new("mc_svc_thread", (GThreadFunc)mc_svc_thread, NULL);

	mc_debug("*** Media Controller Daemon is running ***");

	g_main_loop_run(g_mainloop);

	g_thread_join(svc_thread);
	g_main_loop_unref(g_mainloop);

	mc_debug("*** Media Controller Daemon is stopped ***");

	return 0;
}
