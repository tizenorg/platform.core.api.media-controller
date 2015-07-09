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

#include <string.h>
#include <systemd/sd-daemon.h>

#include "media_controller_svc.h"
#include "media_controller_private.h"
#include "media_controller_socket.h"
#include "media_controller_db_util.h"

static GMainLoop *g_mc_svc_mainloop = NULL;
static int g_connection_cnt = -1;

static int __mc_privilege_ask(int client_sockfd, const char *type, const char *privilege_object)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
#if 0
	ret = security_server_check_privilege_by_sockfd(client_sockfd, type, privilege_object);
	if (ret == SECURITY_SERVER_API_ERROR_ACCESS_DENIED) {
		mc_error("You do not have permission for this operation.");
		ret = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
	} else {
		mc_debug("PERMISSION OK");
	}
#endif
	return ret;
}

static int __create_socket_activation(void)
{
	int fd = -1;
	int listen_fds;

	listen_fds = sd_listen_fds(0);
	if (listen_fds == 1) {
		fd = SD_LISTEN_FDS_START;
		return fd;
	} else if (listen_fds > 1) {
		mc_error("Too many file descriptors received.");
		return -1;
	} else {
		mc_error("There is no socket stream");
		return -1;
	}
}

gboolean _mc_read_service_request_tcp_socket(GIOChannel *src, GIOCondition condition, gpointer data)
{
	int sock = -1;
	int client_sock = -1;
	char *sql_query = NULL;
	mc_comm_msg_s recv_msg;
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	int send_msg = MEDIA_CONTROLLER_ERROR_NONE;
	bool is_duplicated = FALSE;
	unsigned int i = 0;
	mc_svc_data_t *mc_svc_data = (mc_svc_data_t*)data;

	mc_debug("mc_read_service_request_tcp_socket is called!!!!!");

	sock = g_io_channel_unix_get_fd(src);
	if (sock < 0) {
		mc_error("sock fd is invalid!");
		return TRUE;
	}

	/* get client socket fd */
	ret = mc_ipc_accept_client_tcp(sock, &client_sock);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		return TRUE;
	}

	ret = mc_ipc_receive_message_tcp(client_sock, &recv_msg);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("mc_ipc_receive_message_tcp failed [%d]", ret);
		send_msg = ret;
		goto ERROR;
	}

	if (recv_msg.msg_type == MC_MSG_DB_UPDATE) {
		/* Connect media controller DB*/
		if(mc_db_util_connect(&(mc_svc_data->db_handle), recv_msg.uid) != MEDIA_CONTROLLER_ERROR_NONE) {
			mc_error("Failed to connect DB");
			goto ERROR;
		}
		sql_query = strndup(recv_msg.msg, recv_msg.msg_size);
		if (sql_query != NULL) {
			ret = mc_db_util_update_db(mc_svc_data->db_handle, sql_query);
			if (ret != MEDIA_CONTROLLER_ERROR_NONE)
				mc_error("media_db_update_db error : %d", ret);

			send_msg = ret;
			MC_SAFE_FREE(sql_query);
		} else {
			send_msg = MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY;
		}
		/* Disconnect DB*/
		if(mc_db_util_disconnect(mc_svc_data->db_handle) != MEDIA_CONTROLLER_ERROR_NONE) {
			mc_error("Failed to disconnect DB");
		}
	} else if (recv_msg.msg_type == MC_MSG_CLIENT_SET) {
		/* check privileage */
		ret = __mc_privilege_ask(client_sock, "mediacontroller::svc", "w");
		if (ret == MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED) {
			mc_error("permission is denied!");
			send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
			goto ERROR;
		}

		for (i = 0; i < g_list_length(mc_svc_data->mc_svc_list); i++) {
			char *nth_data = (char *)g_list_nth_data(mc_svc_data->mc_svc_list, i);
			if (nth_data != NULL && strcmp(nth_data, recv_msg.msg) == 0) {
				is_duplicated = TRUE;
			}
		}
		if (!is_duplicated) {
			mc_svc_data->mc_svc_list = g_list_append(mc_svc_data->mc_svc_list, recv_msg.msg);
		}
	} else if (recv_msg.msg_type == MC_MSG_CLIENT_GET) {
		/* check privileage */
		ret = __mc_privilege_ask(client_sock, "mediacontroller::svc", "r");
		if (ret == MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED) {
			mc_error("permission is denied!");
			send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
			goto ERROR;
		}

		for (i = 0; i < g_list_length(mc_svc_data->mc_svc_list); i++) {
			send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
			char *nth_data = (char *)g_list_nth_data(mc_svc_data->mc_svc_list, i);
			if (nth_data != NULL && strcmp(nth_data, recv_msg.msg) == 0) {
				mc_svc_data->mc_svc_list = g_list_remove(mc_svc_data->mc_svc_list, nth_data);
				send_msg = MEDIA_CONTROLLER_ERROR_NONE;
			}
		}
	} else if (recv_msg.msg_type == MC_MSG_SERVER_CONNECTION) {
		if((recv_msg.msg_size > 0) && (recv_msg.msg != NULL)) {
			if (strncmp(recv_msg.msg, MC_SERVER_CONNECTION_MSG, recv_msg.msg_size) == 0) {
				if(g_connection_cnt == -1)
					g_connection_cnt = 1;
				else
					g_connection_cnt++;

				mc_error("[No-error] increased connection count [%d]", g_connection_cnt);

				send_msg = MEDIA_CONTROLLER_ERROR_NONE;
			} else {
				mc_error("Wrong message!");
				send_msg = MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
			}
		} else {
			mc_error("Wrong message!");
			send_msg = MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
		}
	} else if (recv_msg.msg_type == MC_MSG_SERVER_DISCONNECTION) {
		if((recv_msg.msg_size > 0) && (recv_msg.msg != NULL)) {
			if (strncmp(recv_msg.msg, MC_SERVER_DISCONNECTION_MSG, recv_msg.msg_size) == 0) {
				g_connection_cnt--;
				mc_error("[No-error] decreased connection count [%d]", g_connection_cnt);

				send_msg = MEDIA_CONTROLLER_ERROR_NONE;
			} else {
				mc_error("Wrong message!");
				send_msg = MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
			}
		} else {
			mc_error("Wrong message!");
			send_msg = MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
		}
	} else {
		mc_error("Wrong message type!");
		send_msg = MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

ERROR:
	if (write(client_sock, &send_msg, sizeof(send_msg)) != sizeof(send_msg)) {
		mc_stderror("send failed");
	} else {
		mc_debug("Sent successfully");
	}

	if (close(client_sock) < 0) {
		mc_stderror("close failed");
	}

	return TRUE;
}

int mc_create_socket_activation(void)
{
	return __create_socket_activation();
}

gboolean mc_svc_thread(void *data)
{
	int sockfd = -1;
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	GSource *source = NULL;
	GIOChannel *channel = NULL;
	GMainContext *context = NULL;
	mc_svc_data_t *mc_svc_data = NULL;

	mc_svc_data = (mc_svc_data_t *)g_malloc(sizeof(mc_svc_data_t));
	if (mc_svc_data == NULL) {
		mc_error("Failed to allocate svc data");
		return FALSE;
	}
	memset(mc_svc_data, 0, sizeof(mc_svc_data_t));

	/* Create TCP Socket*/
	ret = mc_ipc_create_server_socket(MC_DB_UPDATE_PORT, &sockfd);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		/* Disconnect DB*/
		mc_error("Failed to create socket");
		return FALSE;
	}

	/* Create list for command*/
	mc_svc_data->mc_svc_list = g_list_alloc();
	if (mc_svc_data->mc_svc_list == NULL) {
		mc_error("Failed to allocate list");
		close(sockfd);
		return FALSE;
	}

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
	g_source_set_callback(source, (GSourceFunc)_mc_read_service_request_tcp_socket, mc_svc_data, NULL);
	g_source_attach(source, context);

	g_main_context_push_thread_default(context);

	mc_debug("*******************************************");
	mc_debug("*** Media Controller Service thread is running ***");
	mc_debug("*******************************************");

	g_main_loop_run(g_mc_svc_mainloop);

	mc_debug("*** Media Controller Service thread will be closed ***");

	g_io_channel_shutdown(channel,  FALSE, NULL);
	g_io_channel_unref(channel);


	if (mc_svc_data->mc_svc_list != NULL) {
		int i = 0;
		for (i = g_list_length(mc_svc_data->mc_svc_list) - 1; i >= 0; i--) {
			char *nth_data = (char *)g_list_nth_data(mc_svc_data->mc_svc_list, i);
			mc_svc_data->mc_svc_list = g_list_remove(mc_svc_data->mc_svc_list, nth_data);
			MC_SAFE_FREE(nth_data);
		}
		g_list_free(mc_svc_data->mc_svc_list);
	}
	MC_SAFE_FREE(mc_svc_data);

	/*close socket*/
	close(sockfd);

	g_main_loop_unref(g_mc_svc_mainloop);

	return FALSE;
}

GMainLoop *mc_svc_get_main_loop(void)
{
	return g_mc_svc_mainloop;
}

int mc_svc_get_connection_cnt(void)
{
	return g_connection_cnt;
}
