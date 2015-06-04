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
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <security-server.h>

#include "media_controller_private.h"
#include "media_controller_db_util.h"
#include "media_controller_socket.h"

#define MC_SOCK_PATH_PRFX "/tmp/.media_ipc_mc_client"
#define MC_SOCK_PATH_TEMPLATE "XXXXXX"
#define MC_SOCK_PATH MC_SOCK_PATH_PRFX MC_SOCK_PATH_TEMPLATE
#define MC_SOCK_UDP_BLOCK_SIZE 512

static int __mc_privilege_ask(int client_sockfd, char *type, char *privilege_object)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;

	ret = security_server_check_privilege_by_sockfd(client_sockfd, type, privilege_object);
	if (ret == SECURITY_SERVER_API_ERROR_ACCESS_DENIED) {
		mc_error("You do not have permission for this operation.");
		ret = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
	} else {
		mc_debug("PERMISSION OK");
	}

	return ret;
}

int mc_ipc_create_client_socket(int timeout_sec, mc_sock_info_s* sock_info) {
	int sock = -1;

	struct timeval tv_timeout = { timeout_sec, 0 };

	/*Create TCP Socket*/
	if ((sock = socket(PF_FILE, SOCK_STREAM, 0)) < 0) {
		mc_stderror("socket failed");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	if (timeout_sec > 0) {
		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout, sizeof(tv_timeout)) == -1) {
			mc_stderror("setsockopt failed");
			close(sock);
			return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
		}
	}

	sock_info->sock_fd = sock;
	sock_info->sock_path = NULL;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_ipc_delete_client_socket(mc_sock_info_s* sock_info)
{
	int ret = 0;

	close(sock_info->sock_fd);
	mc_debug("sockfd %d close", sock_info->sock_fd);
	if (sock_info->sock_path != NULL) {
		ret = unlink(sock_info->sock_path);
		if (ret< 0) {
			mc_stderror("unlink failed");
		}
		free(sock_info->sock_path);
	}

	return ret;
}

int mc_ipc_create_server_socket(mc_msg_port_type_e port, int *sock_fd) {
	int i;
	bool bind_success = false;
	int sock = -1;
	struct sockaddr_un serv_addr;
	unsigned short serv_port;

	serv_port = port;

	/* Create a TCP socket */
	if ((sock = socket(PF_FILE, SOCK_STREAM, 0)) < 0) {
		mc_stderror("socket failed");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sun_family = AF_UNIX;
	unlink(MC_IPC_PATH[serv_port]);
	strncpy(serv_addr.sun_path, MC_IPC_PATH[serv_port], sizeof(serv_addr.sun_path)-1);

	/* Bind to the local address */
	for (i = 0; i < 20; i ++) {
		if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
			bind_success = true;
			break;
		}
		mc_debug("%d",i);
		usleep(250000);
	}

	if (bind_success == false) {
		mc_stderror("bind failed");
		close(sock);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	mc_debug("bind success");

	/* Listening */
	if (listen(sock, SOMAXCONN) < 0) {
		mc_stderror("listen failed");
		close(sock);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	mc_debug("Listening...");

	/*change permission of sock file*/
	if (chmod(MC_IPC_PATH[serv_port], 0660) < 0)
		mc_stderror("chmod failed");
	if (chown(MC_IPC_PATH[serv_port], 200, 5000) < 0)
		mc_stderror("chown failed");

	*sock_fd = sock;

	return MEDIA_CONTROLLER_ERROR_NONE;

}

int mc_ipc_send_msg_to_client_tcp(int sockfd, mc_comm_msg_s *send_msg, struct sockaddr_un *client_addr) {
	int ret = MEDIA_CONTROLLER_ERROR_NONE;

	if (write(sockfd, send_msg, sizeof(*(send_msg))) != sizeof(*(send_msg))) {
		mc_stderror("sendto failed");
		ret = MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	} else {
		mc_debug("sent result [%d]", send_msg->result);
		mc_debug("result message [%s]", send_msg->msg);
	}

	return ret;
}

int mc_ipc_receive_message_tcp(int client_sock, mc_comm_msg_s *recv_msg) {
	int recv_msg_size = 0;

	if ((recv_msg_size = read(client_sock, recv_msg, sizeof(mc_comm_msg_s))) < 0) {
		if (errno == EWOULDBLOCK) {
			mc_error("Timeout. Can't try any more");
			return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
		} else {
			mc_stderror("recv failed");
			return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
		}
	}

	mc_debug("receive msg from [%d] %d, %s", recv_msg->pid, recv_msg->msg_type, recv_msg->msg);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_ipc_accept_client_tcp(int serv_sock, int* client_sock) {
	int sockfd = -1;
	struct sockaddr_un client_addr;
	socklen_t client_addr_len;

	if (client_sock == NULL)
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;

	client_addr_len = sizeof(client_addr);
	if ((sockfd = accept(serv_sock, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
		mc_stderror("accept failed");
		*client_sock  = -1;
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	*client_sock  = sockfd;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

gboolean mc_read_db_update_tcp_socket(GIOChannel *src, GIOCondition condition, gpointer data) {
	int sock = -1;
	int client_sock = -1;
	char * sql_query = NULL;
	mc_comm_msg_s recv_msg;
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	void *db_handle = (void *)data;
	int send_msg = MEDIA_CONTROLLER_ERROR_NONE;
//	gboolean privilege = TRUE;

	mc_debug("[GD] mc_read_db_update_tcp_socket is called!!!!!");

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
		mc_error("ms_ipc_receive_message_tcp failed [%d]", ret);
		send_msg = ret;
		goto ERROR;
	}

	/* check privileage, it is removed for smack rule */
/*	if(__mc_privilege_check(recv_msg.msg, &privilege) != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("invalid query. size[%d]", recv_msg.msg_size);
		send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
		goto ERROR;
	}

	if (privilege == TRUE) {
		ret = __mc_privilege_ask(client_sock, "mediacontroller::db", "w");
		if (ret == MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED) {
			send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
			goto ERROR;
		}
	}*/

	sql_query = strndup(recv_msg.msg, recv_msg.msg_size);
	if (sql_query != NULL) {
		ret = mc_db_util_update_db(db_handle, sql_query);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE)
			mc_error("media_db_update_db error : %d", ret);

		send_msg = ret;
		MC_SAFE_FREE(sql_query);
	} else {
		send_msg = MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY;
	}

ERROR:
	if (write(client_sock, &send_msg, sizeof(send_msg)) != sizeof(send_msg)) {
		mc_stderror("send failed");
	} else {
		mc_debug("Sent successfully");
	}

	if (close(client_sock) <0) {
		mc_stderror("close failed");
	}

	return TRUE;
}

gboolean mc_read_client_set_tcp_socket(GIOChannel *src, GIOCondition condition, gpointer data) {
	int sock = -1;
	int client_sock = -1;
	mc_comm_msg_s recv_msg;
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	int send_msg = MEDIA_CONTROLLER_ERROR_NONE;
	int i = 0;
	GList *mc_svc_list = (GList*)data;
	bool is_duplicated = FALSE;

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
		mc_error("ms_ipc_receive_message_tcp failed [%d]", ret);
		send_msg = ret;
		goto ERROR;
	}

	/* check privileage */
	ret = __mc_privilege_ask(client_sock, "mediacontroller::svc", "w");
	if (ret == MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED) {
		mc_error("permission is denied!");
		send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
		goto ERROR;
	}

	if (mc_svc_list != NULL && g_list_length(mc_svc_list) == 0) {
		for (i = 0; i < g_list_length(mc_svc_list); i++) {
			char *data = (char*)g_list_nth_data(mc_svc_list, i);
			if (strcmp(data, recv_msg.msg) == 0) {
				is_duplicated = TRUE;
			}
		}
		if (is_duplicated) {
			mc_svc_list = g_list_append(mc_svc_list, recv_msg.msg);
		}
	}

ERROR:
	if (write(client_sock, &send_msg, sizeof(send_msg)) != sizeof(send_msg)) {
		mc_stderror("send failed");
	} else {
		mc_debug("Sent successfully");
	}

	if (close(client_sock) <0) {
		mc_stderror("close failed");
	}

	return TRUE;
}

gboolean mc_read_client_get_tcp_socket(GIOChannel *src, GIOCondition condition, gpointer data) {
	int sock = -1;
	int client_sock = -1;
	mc_comm_msg_s recv_msg;
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	int send_msg = MEDIA_CONTROLLER_ERROR_NONE;
	int i = 0;
	GList *mc_svc_list = (GList*)data;

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
		mc_error("ms_ipc_receive_message_tcp failed [%d]", ret);
		send_msg = ret;
		goto ERROR;
	}

	/* check privileage */
	ret = __mc_privilege_ask(client_sock, "mediacontroller::svc", "r");
	if (ret == MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED) {
		mc_error("permission is denied!");
		send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
		goto ERROR;
	}

	for (i = 0; i < g_list_length(mc_svc_list); i++) {
		send_msg = MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
		char *data = (char*)g_list_nth_data(mc_svc_list, i);
		if (strcmp(data, recv_msg.msg) == 0) {
			mc_svc_list = g_list_remove(mc_svc_list, data);
			send_msg = MEDIA_CONTROLLER_ERROR_NONE;
		}
	}

ERROR:
	if (write(client_sock, &send_msg, sizeof(send_msg)) != sizeof(send_msg)) {
		mc_stderror("send failed");
	} else {
		mc_debug("Sent successfully");
	}

	if (close(client_sock) <0) {
		mc_stderror("close failed");
	}

	return TRUE;
}
