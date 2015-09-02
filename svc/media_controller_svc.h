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

#ifndef _MEDIA_CONTROLLER_SVC_H_
#define _MEDIA_CONTROLLER_SVC_H_

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
	int pid;
	char *data;
} mc_svc_list_t;

typedef struct {
	void *db_handle;
	void *mc_svc_list;
} mc_svc_data_t;

int mc_create_socket_activation(void);
gboolean mc_svc_thread(void *data);
GMainLoop *mc_svc_get_main_loop(void);
int mc_svc_get_connection_cnt(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif/* _MEDIA_CONTROLLER_SVC_H_ */
