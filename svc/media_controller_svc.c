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

#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <string.h>
#include <sys/stat.h>
#include <systemd/sd-daemon.h>

#include "media_controller_svc.h"
#include "media_controller_private.h"
#include "media_controller_socket.h"
#include "media_controller_db_util.h"
#include "media_controller_cynara.h"

static GMainLoop *g_mc_svc_mainloop = NULL;
static int g_connection_cnt = -1;

//////////////////////////////////////////////////////////////////////////////
/// GET ACTIVATE USER ID
//////////////////////////////////////////////////////////////////////////////
#define UID_DBUS_NAME		 "org.freedesktop.login1"
#define UID_DBUS_PATH		 "/org/freedesktop/login1"
#define UID_DBUS_INTERFACE	 UID_DBUS_NAME".Manager"
#define UID_DBUS_METHOD		 "ListUsers"

static int __mc_dbus_get_uid(const char *dest, const char *path, const char *interface, const char *method, uid_t *uid)
{
	DBusConnection *conn = NULL;
	DBusMessage *msg = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iiiter;
	DBusMessageIter iter;
	DBusMessageIter aiter, piter;
	int result = 0;

	int val_int = 0;
	char *val_str = NULL;

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (!conn) {
		mc_error("dbus_bus_get error");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	msg = dbus_message_new_method_call(dest, path, interface, method);
	if (!msg) {
		mc_error("dbus_message_new_method_call(%s:%s-%s)",
		path, interface, method);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	dbus_message_iter_init_append(msg, &iiiter);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
	dbus_message_unref(msg);
	if (!reply) {
		mc_error("dbus_connection_send error(%s:%s) %s %s:%s-%s",
		err.name, err.message, dest, path, interface, method);
		dbus_error_free(&err);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_recurse(&iter, &aiter);

	result = 0;
	while (dbus_message_iter_get_arg_type(&aiter) != DBUS_TYPE_INVALID) {
		result++;
		mc_debug("(%d)th block device information", result);

		dbus_message_iter_recurse(&aiter, &piter);
		dbus_message_iter_get_basic(&piter, &val_int);
		mc_debug("\tType(%d)", val_int);

		dbus_message_iter_next(&piter);
		dbus_message_iter_get_basic(&piter, &val_str);
		mc_debug("\tdevnode(%s)", val_str);

		dbus_message_iter_next(&piter);
		dbus_message_iter_get_basic(&piter, &val_str);
		mc_debug("\tsyspath(%s)", val_str);

		dbus_message_iter_next(&aiter);
	}

	*uid = (uid_t) val_int;

	return result;
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
	int i = 0;
	mc_svc_data_t *mc_svc_data = (mc_svc_data_t*)data;
	mc_peer_creds creds = {0, };

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

	memset(&creds, 0, sizeof(mc_peer_creds));

	ret = mc_cynara_receive_untrusted_message(client_sock, &recv_msg, &creds);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("mc_ipc_receive_message_tcp failed [%d]", ret);
		send_msg = ret;
		goto ERROR;
	}

	if (recv_msg.msg_type == MC_MSG_DB_UPDATE) {
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
	} else if (recv_msg.msg_type == MC_MSG_CLIENT_SET) {
		/* check privileage */
		ret = mc_cynara_check(&creds, MC_CLIENT_PRIVILEGE);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
			mc_error("permission is denied![%d]", ret);
			send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
			goto ERROR;
		}

		MC_SAFE_FREE(creds.uid);
		MC_SAFE_FREE(creds.smack);

		mc_svc_list_t *set_data = (mc_svc_list_t *)malloc(sizeof(mc_svc_list_t));
		set_data->pid = recv_msg.pid;
		set_data->data = strdup(recv_msg.msg);
		mc_svc_data->mc_svc_list = g_list_append(mc_svc_data->mc_svc_list, set_data);
	} else if (recv_msg.msg_type == MC_MSG_CLIENT_GET) {
		send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
		/* check privileage */
		ret = mc_cynara_check(&creds, MC_SERVER_PRIVILEGE);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
			mc_error("permission is denied![%d]", ret);
			goto ERROR;
		}

		MC_SAFE_FREE(creds.uid);
		MC_SAFE_FREE(creds.smack);

		mc_svc_list_t *set_data = NULL;
		for (i = 0; i < (int)g_list_length(mc_svc_data->mc_svc_list); i++) {
			set_data = (mc_svc_list_t *)g_list_nth_data(mc_svc_data->mc_svc_list, i);
			if (set_data != NULL && set_data->data != NULL && strcmp(set_data->data, recv_msg.msg) == 0) {
				mc_svc_data->mc_svc_list = g_list_remove(mc_svc_data->mc_svc_list, set_data);
				MC_SAFE_FREE(set_data->data);
				MC_SAFE_FREE(set_data);
				send_msg = MEDIA_CONTROLLER_ERROR_NONE;
				break;
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

				// remove resource for disconnected process
				mc_svc_list_t *set_data = NULL;
				for (i = (int)(g_list_length(mc_svc_data->mc_svc_list)) - 1; i >= 0; i--) {
					set_data = (mc_svc_list_t *)g_list_nth_data(mc_svc_data->mc_svc_list, i);
					if ((set_data != NULL) && (set_data->pid == recv_msg.pid)) {
						mc_svc_data->mc_svc_list = g_list_remove(mc_svc_data->mc_svc_list, set_data);
						MC_SAFE_FREE(set_data->data);
						MC_SAFE_FREE(set_data);
					}
				}

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
	MC_SAFE_FREE(creds.uid);
	MC_SAFE_FREE(creds.smack);

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
	uid_t uid = -1;

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

	ret = __mc_dbus_get_uid(UID_DBUS_NAME,UID_DBUS_PATH, UID_DBUS_INTERFACE, UID_DBUS_METHOD, &uid);
	if (ret < 0) {
		mc_debug("Failed to send dbus (%d)", ret);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	} else {
		mc_debug("%d get UID[%d]", ret, uid);
	}

	/* Connect media controller DB*/
	if(mc_db_util_connect(&(mc_svc_data->db_handle), uid, true) != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Failed to connect DB");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	/* Destroy tables */
	if (mc_db_util_delete_whole_server_tables(mc_svc_data->db_handle) != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("mc_db_util_delete_whole_server_tables failed [%d]", ret);
	}

	/* Create tables */
	if (mc_db_util_create_tables(mc_svc_data->db_handle) != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("mc_db_util_create_tables failed [%d]", ret);
		return FALSE;
	}

	ret = mc_cynara_enable_credentials_passing(sockfd);
	if(ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Failed to append socket options");
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

	/* Create new channel to watch TCP socket */
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
		unsigned int idx = 0;

		for (idx = 0; idx < g_list_length(mc_svc_data->mc_svc_list); idx++) {
			mc_svc_list_t *set_data = NULL;
			set_data = (mc_svc_list_t *)g_list_nth_data(mc_svc_data->mc_svc_list, idx);
			MC_SAFE_FREE(set_data->data);
			MC_SAFE_FREE(set_data);
		}

		g_list_free(mc_svc_data->mc_svc_list);
	}

	MC_SAFE_FREE(mc_svc_data);

	/* Disconnect media controller DB*/
	if(mc_db_util_disconnect(mc_svc_data->db_handle) != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Failed to connect DB");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

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
