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

#include <stdio.h>
#include <glib.h>
#include <tzplatform_config.h>

#include "media_controller_private.h"

#define MAX_RETRY_COUNT 3
#define MAX_WAIT_COUNT 3
#define MC_SVC_NAME "mediacontroller"

GMainLoop *g_wait_mainloop = NULL;

static gboolean __timeout_func(gpointer data)
{
    mc_debug("Timeout!");
    g_main_loop_quit((GMainLoop *) data);
    return FALSE;
}

static void __wait_for_activate()
{
    int timeout_id = 0;
    g_wait_mainloop = g_main_loop_new(NULL, FALSE);

    timeout_id = g_timeout_add(500, __timeout_func, g_wait_mainloop);
    g_main_loop_run(g_wait_mainloop);
    g_source_remove(timeout_id);
	g_main_loop_unref(g_wait_mainloop);
	g_wait_mainloop = NULL;
}

/* This checks if service daemon is running */
static gboolean __is_service_activated()
{
	gboolean ret = FALSE;
	DIR *pdir;
	struct dirent pinfo;
	struct dirent *result = NULL;

	pdir = opendir("/proc");
	if (pdir == NULL) {
		mc_error("err: NO_DIR");
		return FALSE;
	}

	while (!readdir_r(pdir, &pinfo, &result)) {
		if (result == NULL)
			break;

		if (pinfo.d_type != 4 || pinfo.d_name[0] == '.'
		    || pinfo.d_name[0] > 57)
			continue;

		FILE *fp;
		char buff[128];
		char path[128];

		snprintf(path, sizeof(path), "/proc/%s/status", pinfo.d_name);
		fp = fopen(path, "rt");
		if (fp) {
			if (fgets(buff, 128, fp) == NULL)
				mc_error("fgets failed");
			fclose(fp);

			if (strstr(buff, MC_SVC_NAME)) {
				mc_error("%s proc is already running", buff);
				ret = TRUE;
				break;
			}
		} else {
			mc_error("Can't read file [%s]", path);
		}
	}

	closedir(pdir);

	return ret;
}

static char *__make_key_for_map(const char *main_key, const char *sub_key)
{
	return g_strdup_printf("%s.%s", main_key, sub_key);
}

static void __mc_ipc_signal_cb(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data)
{
	char *key = __make_key_for_map(interface_name, signal_name);
	GList *listener_list = (GList *)user_data;
	unsigned int i = 0;

	mc_debug("__mc_ipc_signal_cb Received : ");

	for (i = 0; i < g_list_length(listener_list); i++) {
		mc_ipc_listener_s *listener = (mc_ipc_listener_s *)g_list_nth_data(listener_list, i);
		if (listener && !strcmp(listener->key, key)) {
			gchar *message = NULL;
			int internal_flags = 0;
			g_variant_get(parameters, "(i&s)", &internal_flags, &message);
			if (!message) {
				mc_error("g_variant_get() fail");
				break;
			}
			mc_debug("Received : [%s] (flags = %x) from %s.%s", message, internal_flags, listener->interface_name, listener->signal_name);
			listener->callback(listener->interface_name, listener->signal_name, message, 0, listener->user_data);
		}
	}
}

static gboolean _mc_ipc_is_listener_duplicated(GList *listener_list, const char *key)
{
	unsigned int i = 0;

	for (i = 0; i < g_list_length(listener_list); i++) {
		mc_ipc_listener_s *listener = (mc_ipc_listener_s *)g_list_nth_data((listener_list), i);
		mc_retvm_if(listener && !strcmp(listener->key, key), TRUE, "listener[%s] is duplicated ", key);
	}

	return FALSE;
}

static guint _mc_ipc_signal_subscribe(GDBusConnection *connection, const char *interface_name, const char *signal_name, void *user_data)
{
	guint ret = g_dbus_connection_signal_subscribe(
	                connection,
	                NULL,
	                interface_name,
	                signal_name,
	                MC_DBUS_PATH,
	                NULL,
	                G_DBUS_SIGNAL_FLAGS_NONE,
	                __mc_ipc_signal_cb,
	                user_data,
	                NULL);

	return ret;
}

static void _mc_ipc_signal_unsubscribe(GDBusConnection *connection, guint handler)
{
	g_dbus_connection_signal_unsubscribe(connection, handler);
}

int mc_ipc_get_dbus_connection(GDBusConnection **connection, int *dref_count)
{
	GError *error = NULL;
	GDBusConnection *_connection = NULL;

	if (*dref_count > 0) {
		mc_retvm_if(!(*connection), MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "__dbus_conn is null");

		mc_error("Reference count : %d", *dref_count);
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (!_connection) {
		mc_error("g_bus_get_sync failed : %s", error ? error->message : "none");
		if (error) g_error_free(error);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	(*connection) = _connection;
	(*dref_count)++;

	mc_debug("Reference count : %d", *dref_count);

	if (error) g_error_free(error);
	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_ipc_unref_dbus_connection(GDBusConnection *connection, int *dref_count)
{
	if (*dref_count > 1) {
		mc_retvm_if(!connection, MEDIA_CONTROLLER_ERROR_INVALID_OPERATION, "__dbus_conn is null");
		(*dref_count)--;
		mc_debug("Reference count : %d", dref_count);
		return MEDIA_CONTROLLER_ERROR_NONE;
	} else if (*dref_count == 1) {
		mc_retvm_if(!connection, MEDIA_CONTROLLER_ERROR_INVALID_OPERATION, "__dbus_conn is null");
		(*dref_count)--;
		mc_debug("Reference count : %d", *dref_count);
	} else {
		mc_retvm_if(connection, MEDIA_CONTROLLER_ERROR_NONE, "__dbus_conn is not null");
		mc_debug("Reference count : %d", *dref_count);
		return MEDIA_CONTROLLER_ERROR_NONE;
	}

	g_object_unref(connection);
	connection = NULL;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_ipc_register_listener(GList *listener_list, GDBusConnection *connection, const char *interface_name, const char *signal_name, mc_signal_received_cb callback, void *user_data)
{
	char *key = NULL;
	guint handler = 0;

	mc_retvm_if(listener_list == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "listener_list is NULL");
	mc_retvm_if(connection == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "connection is NULL");
	mc_retvm_if(interface_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "interface_name is NULL");
	mc_retvm_if(signal_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "signal_name is NULL");
	mc_retvm_if(callback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "callback is NULL");

	key = __make_key_for_map(interface_name, signal_name);

	if (_mc_ipc_is_listener_duplicated(listener_list, key)) {
		mc_error("listener is duplicated");

		MC_SAFE_FREE(key);
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	mc_ipc_listener_s *listener = (mc_ipc_listener_s *)g_malloc(sizeof(mc_ipc_listener_s));
	if (listener == NULL) {
		mc_error("Error memroy allocation");
		MC_SAFE_FREE(key);
		return MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY;
	}
	handler = _mc_ipc_signal_subscribe(connection, interface_name, signal_name, listener_list);
	listener->dbus_conn = connection;
	listener->interface_name = g_strdup(interface_name);
	listener->signal_name = g_strdup(signal_name);
	listener->callback = callback;
	listener->user_data = user_data;
	listener->handler = handler;
	listener->key = key;

	(listener_list) = g_list_append((listener_list), listener);

	mc_debug("listener[%s.%s] is registered", interface_name, signal_name);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_ipc_unregister_listener(GList *listener_list, GDBusConnection *connection, const char *interface_name, const char *signal_name)
{
	int i = 0;
	char *key = NULL;

	mc_retvm_if(listener_list == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "listener_list is NULL");
	mc_retvm_if(connection == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "connection is NULL");
	mc_retvm_if(interface_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "interface_name is NULL");
	mc_retvm_if(signal_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "signal_name is NULL");

	key = __make_key_for_map(interface_name, signal_name);
	if (key == NULL) {
		mc_error("fail to get key");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	for (i = g_list_length(listener_list) - 1; i >= 0; i--) {
		mc_ipc_listener_s *listener = (mc_ipc_listener_s *)g_list_nth_data(listener_list, i);
		if (listener && !strcmp(listener->key, key)) {
			_mc_ipc_signal_unsubscribe(connection, listener->handler);
			MC_SAFE_FREE(listener->interface_name);
			MC_SAFE_FREE(listener->signal_name);
			MC_SAFE_FREE(listener->key);
			(listener_list) = g_list_remove(listener_list, listener);
			MC_SAFE_FREE(listener);
			mc_debug("listener[%s.%s] is unregistered", interface_name, signal_name);
		}
	}

	MC_SAFE_FREE(key);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_ipc_unregister_all_listener(GList *listener_list, GDBusConnection *connection)
{
	int i = 0;

	mc_retvm_if(connection == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "connection is NULL");
	mc_retvm_if(listener_list == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "listener_list is NULL");

	for (i = g_list_length(listener_list) - 1; i >= 0; i--) {
		mc_ipc_listener_s *listener = (mc_ipc_listener_s *)g_list_nth_data(listener_list, i);
		if (listener) {
			_mc_ipc_signal_unsubscribe(connection, listener->handler);
			MC_SAFE_FREE(listener->interface_name);
			MC_SAFE_FREE(listener->signal_name);
			MC_SAFE_FREE(listener->key);
			(listener_list) = g_list_remove(listener_list, listener);
			MC_SAFE_FREE(listener);
		}
	}

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_ipc_send_message(GDBusConnection *connection, const char *dbus_name, const char *interface_name, const char *signal_name, const char *message, int flags)
{
	GError *error = NULL;

	mc_retvm_if(connection == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "connection is NULL");
	mc_retvm_if(signal_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "signal_name is NULL");
	mc_retvm_if(message == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "message is NULL");

	mc_debug("emit signal - interface: %s, signal: %s", interface_name, signal_name);

	gboolean emmiting = g_dbus_connection_emit_signal(
	                        connection,
	                        dbus_name,
	                        MC_DBUS_PATH,
	                        interface_name,
	                        signal_name,
	                        g_variant_new("(is)", 0, message),
	                        &error);
	if (!emmiting) {
		mc_error("g_dbus_connection_emit_signal failed : %s", error ? error->message : "none");
		if (error) {
			mc_error("Error in g_dbus_connection_emit_signal");
			g_error_free(error);
		}
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_ipc_send_message_to_server(mc_msg_type_e msg_type, const char *request_msg)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	int request_msg_size = 0;
	int sockfd = -1;
	mc_sock_info_s sock_info;
	struct sockaddr_un serv_addr;
	int retry_count = 0;

	if (!MC_STRING_VALID(request_msg)) {
		mc_error("invalid query");
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	request_msg_size = strlen(request_msg);
	if (request_msg_size >= MAX_MSG_SIZE) {
		mc_error("Query is Too long. [%d] query size limit is [%d]", request_msg_size, MAX_MSG_SIZE);
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	mc_comm_msg_s send_msg;
	memset((void *)&send_msg, 0, sizeof(mc_comm_msg_s));

	send_msg.msg_type = msg_type;
	send_msg.uid = tzplatform_getuid(TZ_USER_NAME);
	send_msg.msg_size = request_msg_size;
	strncpy(send_msg.msg, request_msg, sizeof(send_msg.msg) - 1);

	/*Create Socket*/
	ret = mc_ipc_create_client_socket(MC_TIMEOUT_SEC_10, &sock_info);
	sockfd = sock_info.sock_fd;
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "socket is not created properly");

	/*Set server Address*/
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, MC_IPC_PATH, sizeof(serv_addr.sun_path) - 1);

	/* Connecting to the media db server */
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		mc_stderror("connect error");
		mc_ipc_delete_client_socket(&sock_info);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	/* Send request */
	if (send(sockfd, &send_msg, sizeof(send_msg), 0) != sizeof(send_msg)) {
		mc_stderror("send failed");
		mc_ipc_delete_client_socket(&sock_info);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	/*Receive Response*/
	int recv_msg_size = -1;
	int recv_msg = -1;
RETRY:
	if ((recv_msg_size = recv(sockfd, &recv_msg, sizeof(recv_msg), 0)) < 0) {
		mc_error("recv failed : [%d]", sockfd);
		mc_stderror("recv failed");

		if (errno == EINTR) {
			mc_stderror("catch interrupt");
			goto RETRY;
		}

		if (errno == EWOULDBLOCK) {
			if (retry_count < MAX_RETRY_COUNT) {
				mc_error("TIME OUT[%d]", retry_count);
				retry_count++;
				goto RETRY;
			}

			mc_ipc_delete_client_socket(&sock_info);
			mc_error("Timeout. Can't try any more");
			return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
		} else {
			mc_stderror("recv failed");
			mc_ipc_delete_client_socket(&sock_info);
			return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
		}
	}

	mc_debug("RECEIVE OK [%d]", recv_msg);
	ret = recv_msg;

	mc_ipc_delete_client_socket(&sock_info);

	return ret;
}

int mc_ipc_service_connect(void)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	int sockfd = -1;
	mc_sock_info_s sock_info;
	struct sockaddr_un serv_addr;
	unsigned int retry = 0;

	/*Create Socket*/
	ret = mc_ipc_create_client_socket(MC_TIMEOUT_SEC_10, &sock_info);
	sockfd = sock_info.sock_fd;
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "socket is not created properly");

	/*Set server Address*/
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, MC_IPC_PATH, sizeof(serv_addr.sun_path) - 1);

	/* Connecting to the media db server */
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		mc_stderror("connect error");
		mc_ipc_delete_client_socket(&sock_info);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	mc_debug("CONNECT OK");

	mc_ipc_delete_client_socket(&sock_info);

	do {
		__wait_for_activate();
	} while((__is_service_activated() == FALSE) && (retry++ < MAX_WAIT_COUNT));

	return ret;
}
