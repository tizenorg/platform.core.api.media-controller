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

#include "media_controller_private.h"
#include "media_controller_db_util.h"
#include "media_controller_socket.h"

#define MC_SOCK_PATH_PRFX "/tmp/.media_ipc_mc_client"
#define MC_SOCK_PATH_TEMPLATE "XXXXXX"
#define MC_SOCK_PATH MC_SOCK_PATH_PRFX MC_SOCK_PATH_TEMPLATE
#define MC_SOCK_UDP_BLOCK_SIZE 512

int mc_ipc_create_client_socket(int timeout_sec, mc_sock_info_s *sock_info)
{
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

int mc_ipc_delete_client_socket(mc_sock_info_s *sock_info)
{
	int ret = 0;

	close(sock_info->sock_fd);
	mc_debug("sockfd %d close", sock_info->sock_fd);
	if (sock_info->sock_path != NULL) {
		ret = unlink(sock_info->sock_path);
		if (ret < 0) {
			mc_stderror("unlink failed");
		}
		free(sock_info->sock_path);
	}

	return ret;
}

int mc_ipc_create_server_socket(mc_msg_port_type_e port, int *sock_fd)
{
	int i = 0;
	bool bind_success = false;
	int sock = -1;
	struct sockaddr_un serv_addr;

	/* Create a TCP socket */
	if ((sock = socket(PF_FILE, SOCK_STREAM, 0)) < 0) {
		mc_stderror("socket failed");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sun_family = AF_UNIX;
	unlink(MC_IPC_PATH);
	strncpy(serv_addr.sun_path, MC_IPC_PATH, sizeof(serv_addr.sun_path) - 1);

	/* Bind to the local address */
	for (i = 0; i < 20; i++) {
		if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0) {
			bind_success = true;
			break;
		}
		mc_debug("%d", i);
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
	if (chmod(MC_IPC_PATH, 0666) < 0)
		mc_stderror("chmod failed");

	*sock_fd = sock;

	return MEDIA_CONTROLLER_ERROR_NONE;

}

int mc_ipc_send_msg_to_client_tcp(int sockfd, mc_comm_msg_s *send_msg, struct sockaddr_un *client_addr)
{
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

int mc_ipc_receive_message_tcp(int client_sock, mc_comm_msg_s *recv_msg)
{
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

int mc_ipc_accept_client_tcp(int serv_sock, int *client_sock)
{
	int sockfd = -1;
	struct sockaddr_un client_addr;
	socklen_t client_addr_len;

	if (client_sock == NULL)
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;

	client_addr_len = sizeof(client_addr);
	if ((sockfd = accept(serv_sock, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
		mc_stderror("accept failed");
		*client_sock  = -1;
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	*client_sock  = sockfd;

	return MEDIA_CONTROLLER_ERROR_NONE;
}
