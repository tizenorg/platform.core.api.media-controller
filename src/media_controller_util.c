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

#include <aul.h>
#include "media_controller_private.h"
#include "media_controller_socket.h"

#define MAX_NAME_LENGTH 255
#define MAX_RETRY_COUNT 3

static void _mc_util_check_valid_name(const char *name, char **new_name)
{
	char old_word[MAX_NAME_LENGTH];
	char new_word[MAX_NAME_LENGTH];
	int i = 0;

	mc_retm_if(name == NULL, "Invalid parameter.");

	memset(old_word, 0, MAX_NAME_LENGTH);
	memset(new_word, 0, MAX_NAME_LENGTH);

	if (strlen(name) > MAX_NAME_LENGTH) {
		memcpy(old_word, name, MAX_NAME_LENGTH);
	} else {
		memcpy(old_word, name, strlen(name));
	}

	/* only 0~9, a~z, A~Z, '.', '_' will be used */
	for (i = 0; i < strlen(old_word); i++) {
		if ((old_word[i] >= '0' && old_word[i] <= '9') ||
		    (old_word[i] >= 'a' && old_word[i] <= 'z') ||
		    (old_word[i] >= 'A' && old_word[i] <= 'Z') ||
		    (old_word[i] == '.' && i != 0)) {
			new_word[i] = old_word[i];
		} else {
			if (i - 1 > 1 && new_word[i - 1] != '.')
				new_word[i] = '_';
			else
				new_word[i] = 'x';
		}
	}

	(*new_name) = strdup(new_word);
	mc_retm_if((*new_name) == NULL, "Error allocation memory.");
}

int mc_util_get_own_name(char **name)
{
	char temp[MAX_NAME_LENGTH];
	int pid = -1;

	pid = getpid();
	if (pid == -1) {
		mc_error("Error failed to get pid!");
	}
	if (AUL_R_OK != aul_app_get_appid_bypid(pid, temp, sizeof(temp))) {
		mc_error("Error failed to get appid!");
	}
	_mc_util_check_valid_name(temp, name);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

char *mc_util_get_interface_name(const char *prefix, const char *type, const char *name)
{
	char *temp = NULL;
	char *interface_name = NULL;

	mc_retvm_if(type == NULL, NULL, "type is NULL");
	mc_retvm_if(name == NULL, NULL, "name is NULL");

	if (prefix == NULL)
	{
		temp = g_strdup_printf("%s.%s.%s", MC_DBUS_INTERFACE_PREFIX, type, name);
	} else {
		temp = g_strdup_printf("%s.%s.%s", prefix, type, name);
	}

	_mc_util_check_valid_name(temp, &interface_name);
	MC_SAFE_FREE(temp);
	return interface_name;
}

int mc_util_set_command_availabe(const char *name, const char *command_type, const char *command)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	int request_msg_size = 0;
	int sockfd = -1;
	mc_sock_info_s sock_info;
	struct sockaddr_un serv_addr;
	int port = MC_DB_SET_PORT;
	int retry_count = 0;
	char *message = NULL;

	if (!MC_STRING_VALID(name) || !MC_STRING_VALID(command_type)) {
		mc_error("invalid query");
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	if (command == NULL)
		message = g_strdup_printf("%s%s", name, command_type);
	else
		message = g_strdup_printf("%s%s%s", name, command_type, command);

	request_msg_size = strlen(message);
	if (request_msg_size >= MAX_MSG_SIZE) {
		mc_error("Query is Too long. [%d] query size limit is [%d]", request_msg_size, MAX_MSG_SIZE);
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	mc_comm_msg_s send_msg;
	memset((void *)&send_msg, 0, sizeof(mc_comm_msg_s));

	send_msg.msg_type = MC_MSG_CLIENT_SET;
	send_msg.msg_size = request_msg_size;
	strncpy(send_msg.msg, message, sizeof(send_msg.msg) - 1);

	/*Create Socket*/
	ret = mc_ipc_create_client_socket(MC_TIMEOUT_SEC_10, &sock_info);
	sockfd = sock_info.sock_fd;
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "socket is not created properly");

	/*Set server Address*/
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, MC_IPC_PATH[port], sizeof(serv_addr.sun_path) - 1);

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
			if (retry_count < MAX_RETRY_COUNT)	{
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

int mc_util_get_command_availabe(const char *name, const char *command_type, const char *command)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	int request_msg_size = 0;
	int sockfd = -1;
	mc_sock_info_s sock_info;
	struct sockaddr_un serv_addr;
	int port = MC_DB_GET_PORT;
	int retry_count = 0;
	char *message = NULL;

	if (!MC_STRING_VALID(name) || !MC_STRING_VALID(command_type)) {
		mc_error("invalid query");
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	if (command == NULL)
		message = g_strdup_printf("%s%s", name, command_type);
	else
		message = g_strdup_printf("%s%s%s", name, command_type, command);

	request_msg_size = strlen(message);
	if (request_msg_size >= MAX_MSG_SIZE) {
		mc_error("Query is Too long. [%d] query size limit is [%d]", request_msg_size, MAX_MSG_SIZE);
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	mc_comm_msg_s send_msg;
	memset((void *)&send_msg, 0, sizeof(mc_comm_msg_s));

	send_msg.msg_type = MC_MSG_CLIENT_GET;
	send_msg.msg_size = request_msg_size;
	strncpy(send_msg.msg, message, sizeof(send_msg.msg) - 1);

	/*Create Socket*/
	ret = mc_ipc_create_client_socket(MC_TIMEOUT_SEC_10, &sock_info);
	sockfd = sock_info.sock_fd;
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "socket is not created properly");

	/*Set server Address*/
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, MC_IPC_PATH[port], sizeof(serv_addr.sun_path) - 1);

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
			if (retry_count < MAX_RETRY_COUNT)	{
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
