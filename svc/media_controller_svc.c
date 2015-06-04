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

#include <unistd.h>
#include "media_controller_db_util.h"
#include "media_controller_private.h"
#include "media_controller_socket.h"
#include "media_controller_svc.h"

GMainLoop *g_mc_svc_mainloop = NULL;
static bool g_mc_svc_thread_ready = FALSE;
static GList *g_mc_svc_list = NULL;

gboolean mc_svc_thread(void *data)
{
	int sockfd = -1;
	int sockfd_set = -1;
	int sockfd_get = -1;
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	GSource *source = NULL;
	GIOChannel *channel = NULL;
	GSource *source_set = NULL;
	GIOChannel *channel_set = NULL;
	GSource *source_get = NULL;
	GIOChannel *channel_get = NULL;
	GMainContext *context = NULL;
	void *db_handle = NULL;

	/* Create TCP Socket*/
	ret = mc_ipc_create_server_socket(MC_DB_UPDATE_PORT, &sockfd);
	if(ret != MEDIA_CONTROLLER_ERROR_NONE) {
		/* Disconnect DB*/
		mc_error("Failed to create socket");
		return FALSE;
	}

	/* Create TCP Socket for set client*/
	ret = mc_ipc_create_server_socket(MC_DB_SET_PORT, &sockfd_set);
	if(ret != MEDIA_CONTROLLER_ERROR_NONE) {
		/* Disconnect DB*/
		close(sockfd);
		mc_error("Failed to create socket");
		return FALSE;
	}

	/* Create TCP Socket for get client*/
	ret = mc_ipc_create_server_socket(MC_DB_GET_PORT, &sockfd_get);
	if(ret != MEDIA_CONTROLLER_ERROR_NONE) {
		/* Disconnect DB*/
		close(sockfd);
		close(sockfd_set);
		mc_error("Failed to create socket");
		return FALSE;
	}

	/* Connect Media DB*/
	if(mc_db_util_connect(&db_handle) != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Failed to connect DB");
		close(sockfd);
		close(sockfd_set);
		close(sockfd_get);
		return FALSE;
	}
	g_mc_svc_list = g_list_alloc ();

	context = g_main_context_new();
	if (context == NULL) {
		mc_error("g_main_context_new failed");
	} else {
		mc_debug("g_main_context_new success");
	}

	/*Init main loop*/
	g_mc_svc_mainloop = g_main_loop_new(context, FALSE);

	/* Create new channel to watch UDP socket */
	channel = g_io_channel_unix_new(sockfd);
	source = g_io_create_watch(channel, G_IO_IN);

	/* Set callback to be called when socket is readable */
	g_source_set_callback(source, (GSourceFunc)mc_read_db_update_tcp_socket, db_handle, NULL);
	g_source_attach(source, context);

	/* Create new channel to watch TCP socket */
	channel_set = g_io_channel_unix_new(sockfd_set);
	source_set = g_io_create_watch(channel_set, G_IO_IN);

	/* Set callback to be called when socket is readable */
	g_source_set_callback(source_set, (GSourceFunc)mc_read_client_set_tcp_socket, g_mc_svc_list, NULL);
	g_source_attach(source_set, context);

	/* Create new channel to watch TCP socket */
	channel_get = g_io_channel_unix_new(sockfd_get);
	source_get = g_io_create_watch(channel_get, G_IO_IN);

	/* Set callback to be called when socket is readable */
	g_source_set_callback(source_get, (GSourceFunc)mc_read_client_get_tcp_socket, g_mc_svc_list, NULL);
	g_source_attach(source_get, context);

	g_main_context_push_thread_default(context);

	mc_debug("*******************************************");
	mc_debug("*** Media Controller Service thread is running ***");
	mc_debug("*******************************************");

	g_mc_svc_thread_ready = TRUE;

	g_main_loop_run(g_mc_svc_mainloop);

	mc_debug("*** Media Controller Service thread will be closed ***");

	g_mc_svc_thread_ready = FALSE;

	g_io_channel_shutdown(channel,  FALSE, NULL);
	g_io_channel_unref(channel);
	g_io_channel_shutdown(channel_set,  FALSE, NULL);
	g_io_channel_unref(channel_set);
	g_io_channel_shutdown(channel_get,  FALSE, NULL);
	g_io_channel_unref(channel_get);

	/* Disconnect DB*/
	mc_db_util_disconnect(db_handle);

	g_list_free(g_mc_svc_list);

	/*close socket*/
	close(sockfd);
	close(sockfd_set);
	close(sockfd_get);

	g_main_loop_unref(g_mc_svc_mainloop);

	return FALSE;
}
