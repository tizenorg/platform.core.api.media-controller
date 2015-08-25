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

#define _GNU_SOURCE

#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <media_controller_cynara.h>
#include <media_controller_type.h>
#include <media_controller_private.h>

#include <cynara-client.h>
#include <cynara-session.h>
#include <cynara-error.h>
#include <cynara-creds-socket.h>

/* this definition is missing in glibc headers (version 2.21). It was introduced in kernel version 2.6.17 */
#ifndef SCM_SECURITY
#define SCM_SECURITY 0x03
#endif

static cynara *_cynara = NULL;
G_LOCK_DEFINE_STATIC(cynara_mutex);

static void mc_cynara_dbg_err(const char *prefix, int error_code)
{
	char error_buffer[256];
	int err;
	error_buffer[0] = '\0';

	err = cynara_strerror(error_code, error_buffer, sizeof(error_buffer));
	if (err == CYNARA_API_SUCCESS) {
		mc_error("%s: %s", prefix, error_buffer);
	} else {
		mc_error("%s: error code %i", prefix, error_code);
	}
}

int mc_cynara_initialize(void)
{
	int ret = cynara_initialize(&_cynara, NULL);
	if (ret != CYNARA_API_SUCCESS) {
		mc_cynara_dbg_err("cynara_initialize", ret);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}
	return MEDIA_CONTROLLER_ERROR_NONE;
}

void mc_cynara_finish(void)
{
	cynara_finish(_cynara);
	_cynara = NULL;
}

int mc_cynara_receive_untrusted_message(int sockfd, mc_comm_msg_s *recv_msg, mc_peer_creds *credentials)
{
	int ret = 0;
	int recv_msg_size = 0;

	if (!recv_msg ||!credentials)
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;

	if ((recv_msg_size = read(sockfd, recv_msg, sizeof(mc_comm_msg_s))) < 0) {
		if (errno == EWOULDBLOCK) {
			mc_error("Timeout. Can't try any more");
			return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
		} else {
			mc_stderror("recv failed");
			return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
		}
	}

/*	mc_debug("receive msg[%d] from [%d(%d)] %d, %s", recv_msg_size, recv_msg->pid, recv_msg->uid, recv_msg->msg_type, recv_msg->msg); */

	ret = cynara_creds_socket_get_pid(sockfd, &(credentials->pid));
	if(ret < 0) {
		mc_error("cynara_creds_socket_get_pid failed");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = cynara_creds_socket_get_user(sockfd, USER_METHOD_UID, &(credentials->uid));
	if(ret < 0) {
		mc_error("cynara_creds_socket_get_user failed");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = cynara_creds_socket_get_client(sockfd, CLIENT_METHOD_SMACK, &(credentials->smack));
	if(ret < 0) {
		mc_error("cynara_creds_socket_get_client failed");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

/*	mc_error("cynara_creds_info : P[%d]U[%s]S[%s]", credentials->pid, credentials->uid, credentials->smack); */

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_cynara_check(const mc_peer_creds *creds, const char *privilege)
{
	int result;
	char *session;

	if (!creds || !privilege)
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;

	session = cynara_session_from_pid(creds->pid);
	if (session == NULL) {
		mc_error("cynara_session_from_pid failed");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	G_LOCK(cynara_mutex);
	result = cynara_check(_cynara, creds->smack, session, creds->uid, privilege);
	G_UNLOCK(cynara_mutex);

	if (result != CYNARA_API_ACCESS_ALLOWED)
		mc_cynara_dbg_err("cynara_check", result);

	MC_SAFE_FREE(session);
	return result == CYNARA_API_ACCESS_ALLOWED ? MEDIA_CONTROLLER_ERROR_NONE: MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED;
}

int mc_cynara_enable_credentials_passing(int sockfd)
{
	const int optval = 1;
	int err = 0;

	err = setsockopt(sockfd, SOL_SOCKET, SO_PASSSEC, &optval, sizeof(optval));
	if (err != 0) {
		mc_error("Failed to set SO_PASSSEC socket option");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	err = setsockopt(sockfd, SOL_SOCKET, SO_PASSCRED, &optval, sizeof(optval));
	if (err != 0) {
		mc_error("Failed to set SO_PASSCRED socket option");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	return MEDIA_CONTROLLER_ERROR_NONE;
}
