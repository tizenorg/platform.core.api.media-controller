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

#ifndef __TIZEN_MEDIA_CONTROLLER_SOCKET_H__
#define __TIZEN_MEDIA_CONTROLLER_SOCKET_H__

#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <tzplatform_config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SERVER_IP			"127.0.0.1"
#define MC_TIMEOUT_SEC_5					5		/**< Response from Server time out */
#define MAX_MSG_SIZE				4096*2
#define MC_SOCK_NOT_ALLOCATE 		-1
//#define MC_SOCK_ACTIVATION_PATH		"/var/run/media-controller/media_sa_controller"
#define MC_IPC_PATH					"/var/run/media-controller/media_ipc_controller"
#define MC_SERVER_CONNECTION_MSG			"Connect"
#define MC_SERVER_DISCONNECTION_MSG		"Disonnect"

typedef enum{
	MC_DB_UPDATE_PORT,		/**< Media Controller DB Update */
	MC_PORT_MAX,
} mc_msg_port_type_e;

typedef enum{
	MC_MSG_DB_UPDATE,		/**< Media Controller DB Update */
	MC_MSG_CLIENT_SET,
	MC_MSG_CLIENT_GET,
	MC_MSG_SERVER_CONNECTION,
	MC_MSG_SERVER_DISCONNECTION,
	MC_MSG_MAX,
} mc_msg_type_e;

typedef struct {
	int sock_fd;
	char *sock_path;
}mc_sock_info_s;

typedef struct {
	mc_msg_type_e msg_type;
	int pid;
	uid_t uid;
	int result;
	size_t msg_size; /*this is size of message below and this does not include the terminationg null byte ('\0'). */
	char msg[MAX_MSG_SIZE];
}mc_comm_msg_s;

int mc_ipc_create_client_socket(int timeout_sec, mc_sock_info_s* sock_info);
int mc_ipc_delete_client_socket(mc_sock_info_s* sock_info);
int mc_ipc_create_server_socket(mc_msg_port_type_e port, int *sock_fd);
int mc_ipc_send_msg_to_client_tcp(int sockfd, mc_comm_msg_s *send_msg, struct sockaddr_un *client_addr);
int mc_ipc_receive_message_tcp(int client_sock, mc_comm_msg_s *recv_msg);
int mc_ipc_accept_client_tcp(int serv_sock, int* client_sock);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_MEDIA_CONTROLLER_SERVER_H__ */
