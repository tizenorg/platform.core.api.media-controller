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

#define MAX_NAME_LENGTH 255

static void _mc_util_check_valid_name(const char *name, char **new_name)
{
	char old_word[MAX_NAME_LENGTH] = {0, };
	char new_word[MAX_NAME_LENGTH] = {0, };
	unsigned int i = 0;

	mc_retm_if(name == NULL, "Invalid parameter.");

	memset(old_word, 0, MAX_NAME_LENGTH);
	memset(new_word, 0, MAX_NAME_LENGTH);

	if (strlen(name) > MAX_NAME_LENGTH)
		memcpy(old_word, name, MAX_NAME_LENGTH);
	else
		memcpy(old_word, name, strlen(name));

	/* only 0~9, a~z, A~Z, '.', '_' will be used */
	for (i = 0; i < strlen(old_word); i++) {
		if ((old_word[i] >= '0' && old_word[i] <= '9') ||
		    (old_word[i] >= 'a' && old_word[i] <= 'z') ||
		    (old_word[i] >= 'A' && old_word[i] <= 'Z') ||
		    (old_word[i] == '.' && i != 0)) {
			new_word[i] = old_word[i];
		} else {
			new_word[i] = 'x';
		}
	}

	(*new_name) = strdup(new_word);

	mc_retm_if((*new_name) == NULL, "Error allocation memory.");
}

int mc_util_get_own_name(char **name)
{
	char temp[MAX_NAME_LENGTH] = {0, };
	int pid = -1;

	memset(temp, 0, MAX_NAME_LENGTH);

	pid = getpid();
	if (pid == -1) {
		mc_error("Error failed to get pid!");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}
	if (AUL_R_OK != aul_app_get_appid_bypid(pid, temp, sizeof(temp))) {
		mc_error("Error failed to get appid!");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	_mc_util_check_valid_name(temp, name);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

char *mc_util_get_interface_name(const char *type, const char *name)
{
	char *temp = NULL;
	char *interface_name = NULL;

	mc_retvm_if(type == NULL, NULL, "type is NULL");
	mc_retvm_if(name == NULL, NULL, "name is NULL");

	temp = g_strdup_printf("%s.%s.%s", MC_DBUS_INTERFACE_PREFIX, type, name);

	_mc_util_check_valid_name(temp, &interface_name);

	if (temp) {
		g_free(temp);
		temp = NULL;
	}

	return interface_name;
}

int mc_util_make_filter_interface_name(const char *prefix, const char *filter, char **interface_name)
{
	char *temp = NULL;

	mc_retvm_if(prefix == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "prefix is NULL");
	mc_retvm_if(filter == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "filter is NULL");

	temp = g_strdup_printf("%s.%s", prefix, filter);

	if (temp == NULL) {
		mc_error("Fail to make interface_name");
		return MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY;
	}

	_mc_util_check_valid_name(temp, interface_name);

	if (temp) {
		g_free(temp);
		temp = NULL;
	}

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_util_set_command_availabe(const char *name, const char *command_type, const char *command)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *message = NULL;

	if (!MC_STRING_VALID(name) || !MC_STRING_VALID(command_type)) {
		mc_error("invalid query");
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	if (command == NULL)
		message = g_strdup_printf("%s%s", name, command_type);
	else
		message = g_strdup_printf("%s%s%s", name, command_type, command);

	ret = mc_ipc_send_message_to_server(MC_MSG_CLIENT_SET, message);

	if (message) {
		g_free(message);
		message = NULL;
	}

	return ret;
}

int mc_util_get_command_availabe(const char *name, const char *command_type, const char *command)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *message = NULL;

	if (!MC_STRING_VALID(name) || !MC_STRING_VALID(command_type)) {
		mc_error("invalid query");
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	if (command == NULL)
		message = g_strdup_printf("%s%s", name, command_type);
	else
		message = g_strdup_printf("%s%s%s", name, command_type, command);

	ret = mc_ipc_send_message_to_server(MC_MSG_CLIENT_GET, message);

	if (message) {
		g_free(message);
		message = NULL;
	}

	return ret;
}
