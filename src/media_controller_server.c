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

#include "media_controller_private.h"
#include "media_controller_db.h"

static int __mc_server_create(media_controller_server_s **mc_server)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *_server = NULL;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	_server = (media_controller_server_s *)calloc(1, sizeof(media_controller_server_s));
	mc_retvm_if(_server == NULL, MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY, "Error allocation memory");

	ret = mc_util_get_own_name(&(_server->server_name));
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Failed to get server_name [%d]", ret);
		goto ERROR;
	}

	_server->metadata = (media_controller_metadata_s *)calloc(1, sizeof(media_controller_metadata_s));
	if (_server->metadata == NULL) {
		mc_error("Error allocation memory");
		ret = MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY;
		goto ERROR;
	}

	ret = mc_ipc_get_dbus_connection(&(_server->dconn), &(_server->dref_count));
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error allocation memory");
		goto ERROR;
	}

	ret = mc_db_connect(&(_server->db_handle), false);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("fail mc_db_connect [%d]", ret);
		goto ERROR;
	}

	_server->listeners = g_list_alloc();
	if (_server->listeners == NULL) {
		ret = MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY;
		mc_error("Error allocation list %d", ret);
		goto ERROR;
	}

	*mc_server = _server;

	return MEDIA_CONTROLLER_ERROR_NONE;

ERROR:
	if (_server->dconn)
		mc_ipc_unref_dbus_connection(_server->dconn, &_server->dref_count);

	if (_server->db_handle)
		mc_db_disconnect(_server->db_handle);

	if (_server->listeners)
		g_list_free(_server->listeners);

	MC_SAFE_FREE(_server->server_name);
	MC_SAFE_FREE(_server->metadata);
	MC_SAFE_FREE(_server);

	return ret;
}

static int __mc_server_destoy(media_controller_server_s *mc_server)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if (mc_server->dconn) {
		ret = mc_ipc_unref_dbus_connection(mc_server->dconn, &mc_server->dref_count);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE)
			mc_error("fail to mc_ipc_unref_dbus_connection");
	}

	if (mc_server->db_handle) {
		ret = mc_db_disconnect(mc_server->db_handle);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE)
			mc_error("fail to mc_db_disconnect");
	}

	if (mc_server->listeners != NULL) {
		g_list_free(mc_server->listeners);
	}

	MC_SAFE_FREE(mc_server->server_name);

	if (mc_server->metadata) {
		MC_SAFE_FREE(mc_server->metadata->title);
		MC_SAFE_FREE(mc_server->metadata->artist);
		MC_SAFE_FREE(mc_server->metadata->album);
		MC_SAFE_FREE(mc_server->metadata->author);
		MC_SAFE_FREE(mc_server->metadata->genre);
		MC_SAFE_FREE(mc_server->metadata->duration);
		MC_SAFE_FREE(mc_server->metadata->date);
		MC_SAFE_FREE(mc_server->metadata->copyright);
		MC_SAFE_FREE(mc_server->metadata->description);
		MC_SAFE_FREE(mc_server->metadata->track_num);
		MC_SAFE_FREE(mc_server->metadata->picture);
		MC_SAFE_FREE(mc_server->metadata);
	}

	MC_SAFE_FREE(mc_server);

	return ret;
}

static void __server_playback_state_command_cb(const char *interface_name, const char *signal_name, const char *message, int size, void *user_data)
{
	gchar **params = NULL;
	media_controller_reciever_s *receiver = (media_controller_reciever_s *)user_data;
	mc_retm_if(receiver == NULL, "reciever is NULL");
	mc_retm_if(message == NULL, "message is NULL");

	mc_server_playback_state_command_received_cb callback = receiver->callback;
	mc_retm_if(receiver->callback == NULL, "playback_state_command_received_cb is NULL");

	mc_debug("__server_playback_state_cb(%s, %s, %s, %d, %p)", interface_name, signal_name, message, size, user_data);

	params = g_strsplit(message, MC_STRING_DELIMITER, 0);
	mc_retm_if(params == NULL, "invalid playback state command");

	if (mc_util_get_command_availabe(params[0], MC_COMMAND_PLAYBACKSTATE, NULL) != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error permission denied");
		return ;
	}

	callback(params[0], (mc_playback_states_e)atoi(params[1]), receiver->user_data);

	g_strfreev(params);
}

static void __server_custom_command_cb(const char *interface_name, const char *signal_name, const char *message, int size, void *user_data)
{
	gchar **params = NULL;
	int enc_size = 0;
	char *sender = NULL;
	char *command = NULL;
	bundle *bundle_data = NULL;

	media_controller_reciever_s *receiver = (media_controller_reciever_s *)user_data;
	mc_retm_if(receiver == NULL, "reciever is NULL");

	mc_server_custom_command_received_cb callback = receiver->callback;
	mc_retm_if(receiver->callback == NULL, "custom_command_received_cb is NULL");

	mc_debug("__server_custom_cb(%s, %s, %s, %d, %p)", interface_name, signal_name, message, size, user_data);

	params = g_strsplit(message, MC_STRING_DELIMITER, 0);
	mc_retm_if(params == NULL, "invalid custom data");

	if(params[0])
		sender = strdup(params[0]);

	if (mc_util_get_command_availabe(sender, MC_COMMAND_CUSTOM, params[1]) != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error permission denied");
		MC_SAFE_FREE(sender);
		g_strfreev(params);
		return ;
	}

	command = strdup(params[1]);
	enc_size = atoi(params[2]);
	if (enc_size > 0)
		bundle_data = bundle_decode((bundle_raw *)params[3], enc_size);

	callback(sender, command, bundle_data, receiver->user_data);

	g_strfreev(params);
}

static int __mc_server_current_is_latest(media_controller_server_s *mc_server, bool *is_latest)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *latest_server_name = NULL;

	*is_latest = FALSE;

	ret = mc_db_get_latest_server_name(mc_server->db_handle, &latest_server_name);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("fail mc_db_get_latest_server_name [%d]", ret);
		return ret;
	}

	if ((latest_server_name != NULL) && (mc_server->server_name != NULL)) {
		if (!strcmp(latest_server_name, mc_server->server_name))
			*is_latest = TRUE;
	}

	MC_SAFE_FREE(latest_server_name);
	return MEDIA_CONTROLLER_ERROR_NONE;
}

static int __mc_server_send_message(media_controller_server_s *mc_server, const char *interface_name, const char *signal_name, int param1, unsigned long long param2)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *message = NULL;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(interface_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "interface_name is NULL");
	mc_retvm_if(signal_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "signal_name is NULL");

	if (!strcmp(MC_DBUS_SIGNAL_NAME_PLAY_BACK, signal_name)) {
		message = g_strdup_printf("%s%s%d%s%llu", mc_server->server_name, MC_STRING_DELIMITER, param1, MC_STRING_DELIMITER, param2);
	} else if (!strcmp(MC_DBUS_SIGNAL_NAME_METADATA, signal_name)) {
		message = g_strdup_printf("%s", mc_server->server_name);
	} else {
		message = g_strdup_printf("%s%s%d", mc_server->server_name, MC_STRING_DELIMITER, param1);
	}

	if (message == NULL) {
		mc_error("Error when making message");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = mc_ipc_send_message(mc_server->dconn, NULL, interface_name, signal_name, message, 0);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error mc_ipc_send_message [%d]", ret);
	}

	mc_debug("interface_name[%s] signal_name[%s] message[%s]", interface_name, signal_name, message);

	MC_SAFE_FREE(message);

	return ret;
}

int mc_server_set_playback_state(mc_server_h server, mc_playback_states_e state)
{
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(((state < MEDIA_PLAYBACK_STATE_PLAYING) || (state > MEDIA_PLAYBACK_STATE_REWIND)), MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "state is invalid");

	mc_debug("playback state update [%d]", state);

	mc_server->playback.state = state;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_server_set_playback_position(mc_server_h server, unsigned long long position)
{
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	mc_debug("playback position update [%llu]", position);

	mc_server->playback.position = position;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_server_update_playback_info(mc_server_h server)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	mc_debug("playback info update: state [%d], position [%llu]", mc_server->playback.state, mc_server->playback.position);

	ret = mc_db_update_playback_info(mc_server->db_handle, mc_server->server_name, (int)mc_server->playback.state, mc_server->playback.position);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("fail mc_db_update_playback_info [%d]", ret);
		return ret;
	}

	ret = __mc_server_send_message(mc_server, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_PLAY_BACK, mc_server->playback.state, mc_server->playback.position);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error __mc_server_send_message [%d]", ret);
	}

	if (mc_server->playback.state == MEDIA_PLAYBACK_STATE_PLAYING) {
		ret = mc_db_update_latest_server_table(mc_server->db_handle, mc_server->server_name);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
			mc_error("fail mc_db_update_playback_info [%d]", ret);
			return ret;
		}
	}

	return ret;
}

int mc_server_update_shuffle_mode(mc_server_h server, mc_shuffle_mode_e mode)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if ((mode != SHUFFLE_MODE_ON) && (mode != SHUFFLE_MODE_OFF)) {
		mc_error("Invalid shuffle mode [%d]", mode);
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	mc_debug("shuffle mode %d", mode);

	ret = mc_db_update_shuffle_mode(mc_server->db_handle, mc_server->server_name, mode);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail to mc_db_update_shuffle_mode");

	ret = __mc_server_send_message(mc_server, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_PLAYBACK_SHUFFLE, mode, 0);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail to __mc_server_send_message");

	return ret;
}

int mc_server_update_repeat_mode(mc_server_h server, mc_repeat_mode_e mode)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if ((mode != REPEAT_MODE_ON) && (mode != REPEAT_MODE_OFF)) {
		mc_error("Invalid repeat mode [%d]", mode);
		return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	mc_debug("repeat mode %d", mode);

	ret = mc_db_update_repeat_mode(mc_server->db_handle, mc_server->server_name, mode);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail to mc_db_update_repeat_mode");

	ret = __mc_server_send_message(mc_server, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_PLAYBACK_REPEAT, mode, 0);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail to __mc_server_send_message");

	return ret;
}

int mc_server_set_metadata(mc_server_h server, mc_meta_e attribute, const char *value)
{
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	mc_debug("meta data set attribute [%d] value [%s]", attribute, value);

	switch (attribute) {
		case MEDIA_TITLE:
			MC_SAFE_FREE(mc_server->metadata->title);
			if (value != NULL)
				mc_server->metadata->title = strdup(value);
			break;
		case MEDIA_ARTIST:
			MC_SAFE_FREE(mc_server->metadata->artist);
			if (value != NULL)
				mc_server->metadata->artist = strdup(value);
			break;
		case MEDIA_ALBUM:
			MC_SAFE_FREE(mc_server->metadata->album);
			if (value != NULL)
				mc_server->metadata->album = strdup(value);
			break;
		case MEDIA_AUTHOR:
			MC_SAFE_FREE(mc_server->metadata->author);
			if (value != NULL)
				mc_server->metadata->author = strdup(value);
			break;
		case MEDIA_GENRE:
			MC_SAFE_FREE(mc_server->metadata->genre);
			if (value != NULL)
				mc_server->metadata->genre = strdup(value);
			break;
		case MEDIA_DURATION:
			MC_SAFE_FREE(mc_server->metadata->duration);
			if (value != NULL)
				mc_server->metadata->duration = strdup(value);
			break;
		case MEDIA_DATE:
			MC_SAFE_FREE(mc_server->metadata->date);
			if (value != NULL)
				mc_server->metadata->date = strdup(value);
			break;
		case MEDIA_COPYRIGHT:
			MC_SAFE_FREE(mc_server->metadata->copyright);
			if (value != NULL)
				mc_server->metadata->copyright = strdup(value);
			break;
		case MEDIA_DESCRIPTION:
			MC_SAFE_FREE(mc_server->metadata->description);
			if (value != NULL)
				mc_server->metadata->description = strdup(value);
			break;
		case MEDIA_TRACK_NUM:
			MC_SAFE_FREE(mc_server->metadata->track_num);
			if (value != NULL)
				mc_server->metadata->track_num = strdup(value);
			break;
		case MEDIA_PICTURE:
			MC_SAFE_FREE(mc_server->metadata->picture);
			if (value != NULL)
				mc_server->metadata->picture = strdup(value);
			break;
		default:
			mc_error("Invalid Parameter [%d]", attribute);
			return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_server_update_metadata(mc_server_h server)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mc_db_update_whole_metadata(mc_server->db_handle, mc_server->server_name,
	                                  mc_server->metadata->title, mc_server->metadata->artist, mc_server->metadata->album, mc_server->metadata->author,
	                                  mc_server->metadata->genre, mc_server->metadata->duration, mc_server->metadata->date, mc_server->metadata->copyright,
	                                  mc_server->metadata->description, mc_server->metadata->track_num, mc_server->metadata->picture);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail to mc_db_update_whole_metadata");

	ret = __mc_server_send_message(mc_server, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_METADATA, 0, 0);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail to __mc_server_send_message");

	return ret;
}

int mc_server_set_playback_state_command_received_cb(mc_server_h server, mc_server_playback_state_command_received_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(callback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "callback is NULL");

	mc_server->playback_state_reciever.callback = callback;
	mc_server->playback_state_reciever.user_data = user_data;

	char *interface_name = mc_util_get_interface_name(MC_SERVER, mc_server->server_name);
	ret = mc_ipc_register_listener(mc_server->listeners, mc_server->dconn, interface_name,
	                               MC_DBUS_SIGNAL_NAME_PLAYBACK_STATE_CMD, __server_playback_state_command_cb, (void *)&(mc_server->playback_state_reciever));

	MC_SAFE_FREE(interface_name);

	return ret;
}

int mc_server_unset_playback_state_command_received_cb(mc_server_h server)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	char *interface_name = mc_util_get_interface_name(MC_SERVER, mc_server->server_name);
	ret = mc_ipc_unregister_listener(mc_server->listeners, mc_server->dconn, interface_name,
	                                 MC_DBUS_SIGNAL_NAME_PLAYBACK_STATE_CMD);

	mc_server->playback_state_reciever.callback = NULL;
	mc_server->playback_state_reciever.user_data = NULL;

	MC_SAFE_FREE(interface_name);

	return ret;
}

int mc_server_set_custom_command_received_cb(mc_server_h server, mc_server_custom_command_received_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(callback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "callback is NULL");

	mc_server->custom_cmd_reciever.callback = callback;
	mc_server->custom_cmd_reciever.user_data = user_data;

	char *interface_name = mc_util_get_interface_name(MC_SERVER, mc_server->server_name);
	ret = mc_ipc_register_listener(mc_server->listeners, mc_server->dconn, interface_name,
	                               MC_DBUS_SIGNAL_NAME_CUSTOM_CMD, __server_custom_command_cb, (void *)&(mc_server->custom_cmd_reciever));

	MC_SAFE_FREE(interface_name);

	return ret;
}

int mc_server_unset_custom_command_received_cb(mc_server_h server)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	char *interface_name = mc_util_get_interface_name(MC_SERVER, mc_server->server_name);
	ret = mc_ipc_unregister_listener(mc_server->listeners, mc_server->dconn, interface_name, MC_DBUS_SIGNAL_NAME_CUSTOM_CMD);

	mc_server->custom_cmd_reciever.callback = NULL;
	mc_server->custom_cmd_reciever.user_data = NULL;

	MC_SAFE_FREE(interface_name);

	return ret;
}

int mc_server_send_command_reply(mc_server_h server, const char *client_name, int result_code, bundle *data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *message = NULL;
	int size_r = 0;
	bundle_raw *raw_data = NULL;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(client_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "client_name is NULL");

	if (data) {
		ret = bundle_encode(data, &raw_data, &size_r);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
			mc_error("Error while encoding bundle [%d]", ret);
			return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
		}
	}

	message = g_strdup_printf("%s%s%d%s%d%s%s", mc_server->server_name, MC_STRING_DELIMITER, result_code, MC_STRING_DELIMITER, size_r, MC_STRING_DELIMITER, (unsigned char *)raw_data);

	char *interface_name = mc_util_get_interface_name(MC_CLIENT, client_name);
	ret = mc_ipc_send_message(mc_server->dconn, NULL, interface_name, MC_DBUS_SIGNAL_NAME_CMD_REPLY, message, 0);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE)
		mc_error("fail mc_ipc_send_message [%d]", ret);

	MC_SAFE_FREE(message);
	MC_SAFE_FREE(interface_name);

	return ret;
}

int mc_server_create(mc_server_h *server)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = NULL;
	bool table_exist = FALSE;
	bool server_registerd = FALSE;

	mc_debug_fenter();

	mc_retvm_if(server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is null");

	/*Try Socket Activation by systemd*/
	ret = mc_ipc_service_connect();
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Failed to get mc_ipc_service_connect [%d]", ret);
		return ret;
	}

	ret = __mc_server_create(&mc_server);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("fail __mc_server_create [%d]", ret);
		__mc_server_destoy(mc_server);
		return ret;
	}

	ret = mc_db_create_tables(mc_server->db_handle);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("mc_db_create_tables failed [%d]", ret);
	}

	ret = mc_db_check_server_registerd(mc_server->db_handle, mc_server->server_name, &server_registerd);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("mc_db_check_server_table_exist failed [%d]", ret);
		__mc_server_destoy(mc_server);
		return ret;
	}

	if (server_registerd) {
		mc_error("Already registered server");
		__mc_server_destoy(mc_server);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = mc_db_check_server_table_exist(mc_server->db_handle, mc_server->server_name, &table_exist);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("mc_db_check_server_table_exist failed [%d]", ret);
		__mc_server_destoy(mc_server);
		return ret;
	}

	if (table_exist) {
		ret = mc_db_delete_server_table(mc_server->db_handle, mc_server->server_name);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
			mc_error("mc_db_delete_server_table failed [%d]", ret);
			__mc_server_destoy(mc_server);
			return ret;
		}

		ret = mc_db_delete_server_address_from_table(mc_server->db_handle, MC_DB_TABLE_SERVER_LIST, mc_server->server_name);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
			mc_error("mc_db_delete_server_address_from_table failed [%d]", ret);
			__mc_server_destoy(mc_server);
			return ret;
		}
	}

	ret = mc_db_create_server_table(mc_server->db_handle, mc_server->server_name);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("fail mc_db_clear_table [%d]", ret);
		__mc_server_destoy(mc_server);
		return ret;
	}

	ret = mc_db_insert_server_address_into_table(mc_server->db_handle, mc_server->server_name, mc_server->server_name);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("fail mc_db_insert_server_address_into_table [%d]", ret);
		__mc_server_destoy(mc_server);
		return ret;
	}

	ret = mc_db_insert_server_address_into_table(mc_server->db_handle, MC_DB_TABLE_SERVER_LIST, mc_server->server_name);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("fail mc_db_insert_server_address_into_table [%d]", ret);
		__mc_server_destoy(mc_server);
		return ret;
	}

	ret = __mc_server_send_message(mc_server, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_SERVER_STATE, MC_SERVER_STATE_ACTIVATE, 0);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error __mc_server_send_message [%d]", ret);
	}

	*server = (mc_server_h)mc_server;

	mc_debug_fleave();

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_server_destroy(mc_server_h server)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	bool is_latest = FALSE;
	media_controller_server_s *mc_server = (media_controller_server_s *)server;

	mc_debug_fenter();

	mc_retvm_if(mc_server == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mc_ipc_unregister_all_listener(mc_server->listeners, mc_server->dconn);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("fail mc_ipc_unregister_all_listener [%d]", ret);
	}

	ret = mc_db_delete_server_address_from_table(mc_server->db_handle, MC_DB_TABLE_SERVER_LIST, mc_server->server_name);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE)
		mc_error("fail mc_db_delete_server_address_from_table [%d]", ret);

	ret = __mc_server_current_is_latest(mc_server, &is_latest);
	if (is_latest) {
		ret = mc_db_update_server_state(mc_server->db_handle, mc_server->server_name, MC_SERVER_STATE_DEACTIVATE);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE)
			mc_error("fail mc_db_delete_server_table [%d]", ret);
	} else {
		ret = mc_db_delete_server_table(mc_server->db_handle, mc_server->server_name);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE)
			mc_error("fail mc_db_delete_server_table [%d]", ret);
	}

	ret = __mc_server_send_message(mc_server, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_SERVER_STATE, MC_SERVER_STATE_DEACTIVATE, 0);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error __mc_server_send_message [%d]", ret);
	}

	/*Send Disconnection Msg to Server*/
	ret = mc_ipc_send_message_to_server(MC_MSG_SERVER_DISCONNECTION, MC_SERVER_DISCONNECTION_MSG);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Failed to mc_ipc_send_message_to_server [%d]", ret);
	}

	ret = __mc_server_destoy(mc_server);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE)
		mc_error("fail __mc_server_destoy [%d]", ret);

	mc_debug_fleave();

	return ret;
}
