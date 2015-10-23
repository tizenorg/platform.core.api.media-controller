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

#ifndef __TIZEN_MEDIA_CONTROLLER_CYNARA_H__
#define __TIZEN_MEDIA_CONTROLLER_CYNARA_H__

#include <stdbool.h>

#include <media_controller_private.h>

#define MC_CLIENT_PRIVILEGE "http://tizen.org/privilege/mediacontroller.client"
#define MC_SERVER_PRIVILEGE "http://tizen.org/privilege/mediacontroller.server"

typedef struct {
	pid_t pid;
	char *uid;
	char *smack;
} mc_peer_creds;

int mc_cynara_initialize(void);
int mc_cynara_check(const mc_peer_creds *creds, const char *privilege);
int mc_cynara_receive_untrusted_message(int sockfd, mc_comm_msg_s *recv_msg, mc_peer_creds *credentials);
int mc_cynara_enable_credentials_passing(int sockfd);
void mc_cynara_finish(void);


#endif/* _MEDIA_CONTROLLER_CYNARA_H_ */
