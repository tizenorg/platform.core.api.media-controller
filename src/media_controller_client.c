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

#include "media_controller_client.h"
#include "media_controller_private.h"
#include "media_controller_db.h"


static void __client_server_cb(char *interface_name, char *signal_name, char *message, int size, void *user_data)
{
	gchar **params;
	media_controller_reciever_s *reciever = (media_controller_reciever_s *)user_data;
	mc_server_state_updated_cb callback = (mc_server_state_updated_cb)reciever->callback;

	mc_retm_if(reciever == NULL, "reciever is NULL");
	mc_retm_if(reciever->callback == NULL, "server_state_cb is NULL");
	mc_retm_if(message == NULL, "message is NULL");

	mc_debug("__client_server_cb(%s, %s, %s, %d, %p)", interface_name, signal_name, message, size, user_data);

	params = g_strsplit(message, MC_STRING_DELIMITER, 0);
	callback(params[0], (mc_server_state_e)atoi(params[1]), reciever->user_data);

	g_strfreev(params);
}

static void __client_playback_cb(char *interface_name, char *signal_name, char *message, int size, void *user_data)
{
	gchar **params;
	media_controller_reciever_s *reciever = (media_controller_reciever_s *)user_data;
	mc_playback_updated_cb callback = (mc_playback_updated_cb)reciever->callback;
	media_controller_playback_s *playback = NULL;

	mc_retm_if(reciever == NULL, "reciever is NULL");
	mc_retm_if(reciever->callback == NULL, "playback_cb is NULL");
	mc_retm_if(message == NULL, "message is NULL");

	mc_debug("__client_playback_cb(%s, %s, %s, %d, %p)", interface_name, signal_name, message, size, user_data);

	playback = (media_controller_playback_s *)g_malloc(sizeof(media_controller_playback_s));

	params = g_strsplit(message, MC_STRING_DELIMITER, 0);
	playback->state = atoi(params[1]);
	playback->position = atol(params[2]);

	callback(params[0], (mc_playback_h) playback, reciever->user_data);

	g_strfreev(params);
}

static void __client_metadata_cb(char *interface_name, char *signal_name, char *message, int size, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	mc_metadata_h metadata = NULL;
	media_controller_client_s *mc_client = (media_controller_client_s *)user_data;
	mc_metadata_updated_cb callback = (mc_metadata_updated_cb)mc_client->metadata_cb.callback;

	mc_retm_if(mc_client == NULL, "mc_client is NULL");
	mc_retm_if(mc_client->metadata_cb.callback == NULL, "metadata_cb is NULL");
	mc_retm_if(message == NULL, "message is NULL");

	mc_debug("__client_metadata_cb(%s, %s, %s, %d, %p)", interface_name, signal_name, message, size, user_data);

	ret = mc_db_get_metadata_info(mc_client->db_handle, message, &metadata);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE)
		mc_error("Fail to mc_db_get_metadata_info");

	callback(message, metadata, mc_client->metadata_cb.user_data);

	mc_client_destroy_metadata(metadata);
}

static void __client_shuffle_cb(char *interface_name, char *signal_name, char *message, int size, void *user_data)
{
	gchar **params;
	media_controller_reciever_s *reciever = (media_controller_reciever_s *)user_data;
	mc_shuffle_mode_changed_cb callback = (mc_shuffle_mode_changed_cb)reciever->callback;

	mc_retm_if(reciever == NULL, "reciever is NULL");
	mc_retm_if(reciever->callback == NULL, "shuffle_cb is NULL");

	mc_debug("__client_shuffle_cb(%s, %s, %s, %d, %p)", interface_name, signal_name, message, size, user_data);

	params = g_strsplit(message, MC_STRING_DELIMITER, 0);
	callback(params[0], (mc_shuffle_mode_e)atoi(params[1]), reciever->user_data);

	g_strfreev(params);
}

static void __client_repeat_cb(char *interface_name, char *signal_name, char *message, int size, void *user_data)
{
	gchar **params;
	media_controller_reciever_s *reciever = (media_controller_reciever_s *)user_data;
	mc_repeat_mode_changed_cb callback = (mc_repeat_mode_changed_cb)reciever->callback;

	mc_retm_if(reciever == NULL, "reciever is NULL");
	mc_retm_if(reciever->callback == NULL, "repeat_cb is NULL");

	mc_debug("__client_repeat_cb(%s, %s, %s, %d, %p)", interface_name, signal_name, message, size, user_data);

	params = g_strsplit(message, MC_STRING_DELIMITER, 0);
	callback(params[0], (mc_repeat_mode_e)atoi(params[1]), reciever->user_data);

	g_strfreev(params);
}

static void __client_reply_cb(char *interface_name, char *signal_name, char *message, int size, void *user_data)
{
	gchar **params = NULL;
	int enc_size = 0;
	bundle *bundle_data = NULL;

	media_controller_reciever_s *reciever = (media_controller_reciever_s *)user_data;
	mc_retm_if(reciever == NULL, "reciever is NULL");

	mc_command_reply_received_cb callback = (mc_command_reply_received_cb)reciever->callback;
	mc_retm_if(reciever->callback == NULL, "reply_cb is NULL");

	mc_debug("__client_reply_cb(%s, %s, %s, %d, %p)", interface_name, signal_name, message, size, user_data);

	params = g_strsplit(message, MC_STRING_DELIMITER, 0);
	mc_retm_if(params == NULL, "invalid custom data");

	enc_size = atoi(params[2]);
	if (enc_size > 0)
		bundle_data = bundle_decode((bundle_raw *)params[3], enc_size);

	callback(params[0], atoi(params[1]), bundle_data, reciever->user_data);

	g_strfreev(params);
}

static int __mc_client_destroy(media_controller_client_s *mc_client)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	if (mc_client->dconn) {
		ret = mc_ipc_unref_dbus_connection(mc_client->dconn, &mc_client->dref_count);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE)
			mc_error("fail to mc_ipc_unref_dbus_connection");
	}

	if (mc_client->db_handle) {
		ret = mc_db_disconnect(mc_client->db_handle);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE)
			mc_error("fail to mc_db_disconnect");
	}

	MC_SAFE_FREE(mc_client->client_name);
	MC_SAFE_FREE(mc_client);

	return ret;
}


int mc_client_create(mc_client_h *client)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = NULL;

	mc_retvm_if(client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	mc_client = (media_controller_client_s *)calloc(1, sizeof(media_controller_client_s));
	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY, "Error allocation memory");

	mc_client->listeners = g_list_alloc();
	if (mc_client->listeners == NULL) {
		ret = MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY;
		mc_error("Error allocation list %d", ret);
		__mc_client_destroy(mc_client);
		return ret;
	}

	ret = mc_util_get_own_name(&(mc_client->client_name));
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Filed to get client name %d", ret);
		__mc_client_destroy(mc_client);
		return ret;
	}

	ret = mc_ipc_get_dbus_connection(&(mc_client->dconn), &(mc_client->dref_count));
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("error in client init %d", ret);
		__mc_client_destroy(mc_client);
		return ret;
	}

	ret = mc_db_connect(&mc_client->db_handle);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("error in connecting to DB %d", ret);
		__mc_client_destroy(mc_client);
		return ret;
	}

	ret = mc_db_create_tables(mc_client->db_handle);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("mc_db_create_tables failed %d", ret);
		__mc_client_destroy(mc_client);
		return ret;
	}

	*client = (mc_client_h)mc_client;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_client_set_server_update_cb(mc_client_h client, mc_server_state_updated_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(callback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "callback is NULL");

	mc_client->server_state_cb.callback = callback;
	mc_client->server_state_cb.user_data = user_data;

	ret = mc_ipc_register_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_SERVER_STATE,
	                               __client_server_cb, (void *)&(mc_client->server_state_cb));

	return ret;
}

int mc_client_unset_server_update_cb(mc_client_h client)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mc_ipc_unregister_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_SERVER_STATE);

	mc_client->server_state_cb.callback = NULL;
	mc_client->server_state_cb.user_data = NULL;

	return ret;
}

int mc_client_set_playback_update_cb(mc_client_h client, mc_playback_updated_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(callback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "callback is NULL");

	mc_client->playback_cb.callback = callback;
	mc_client->playback_cb.user_data = user_data;

	ret = mc_ipc_register_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_PLAY_BACK	,
	                               __client_playback_cb, (void *)&(mc_client->playback_cb));

	return ret;
}

int mc_client_unset_playback_update_cb(mc_client_h client)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mc_ipc_unregister_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_PLAY_BACK);

	mc_client->playback_cb.callback = NULL;
	mc_client->playback_cb.user_data = NULL;

	return ret;
}

int mc_client_set_metadata_update_cb(mc_client_h client, mc_metadata_updated_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(callback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "callback is NULL");

	mc_client->metadata_cb.callback = callback;
	mc_client->metadata_cb.user_data = user_data;

	ret = mc_ipc_register_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_METADATA,
	                               __client_metadata_cb, (void *)(mc_client));

	return ret;
}

int mc_client_unset_metadata_update_cb(mc_client_h client)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mc_ipc_unregister_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_METADATA);

	mc_client->metadata_cb.callback = NULL;
	mc_client->metadata_cb.user_data = NULL;

	return ret;
}

int mc_client_set_shuffle_mode_update_cb(mc_client_h client, mc_shuffle_mode_changed_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(callback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "callback is NULL");

	mc_client->shuffle_cb.callback = callback;
	mc_client->shuffle_cb.user_data = user_data;

	ret = mc_ipc_register_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_PLAYBACK_SHUFFLE,
	                               __client_shuffle_cb, (void *)&(mc_client->shuffle_cb));

	return ret;
}

int mc_client_unset_shuffle_mode_update_cb(mc_client_h client)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mc_ipc_unregister_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_PLAYBACK_SHUFFLE);

	mc_client->shuffle_cb.callback = NULL;
	mc_client->shuffle_cb.user_data = NULL;

	return ret;
}

int mc_client_set_repeat_mode_update_cb(mc_client_h client, mc_repeat_mode_changed_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(callback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "callback is NULL");

	mc_client->repeat_cb.callback = callback;
	mc_client->repeat_cb.user_data = user_data;

	ret = mc_ipc_register_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_PLAYBACK_REPEAT,
	                               __client_repeat_cb, (void *)&(mc_client->repeat_cb));

	return ret;
}

int mc_client_unset_repeat_mode_update_cb(mc_client_h client)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mc_ipc_unregister_listener(mc_client->listeners, mc_client->dconn, MC_DBUS_UPDATE_INTERFACE, MC_DBUS_SIGNAL_NAME_PLAYBACK_REPEAT);

	mc_client->repeat_cb.callback = NULL;
	mc_client->repeat_cb.user_data = NULL;

	return ret;
}

int mc_client_get_playback_state(mc_playback_h playback, mc_playback_states_e *state)
{
	media_controller_playback_s *mc_playback = (media_controller_playback_s *)playback;

	mc_retvm_if(mc_playback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(state == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "state is NULL");

	*state = mc_playback->state;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_client_get_playback_position(mc_playback_h playback, unsigned long long *position)
{
	media_controller_playback_s *mc_playback = (media_controller_playback_s *)playback;

	mc_retvm_if(mc_playback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(position == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "position is NULL");

	*position = mc_playback->position;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_client_get_metadata(mc_metadata_h metadata, mc_meta_e attribute, char **value)
{
	char *meta_val = NULL;
	media_controller_metadata_s *mc_metadata = (media_controller_metadata_s *)metadata;

	mc_retvm_if(mc_metadata == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(value == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "value is NULL");

	*value = NULL;

	mc_debug("attribute[%d]", attribute);

	switch (attribute) {
		case MEDIA_TITLE:
			if (mc_metadata->title != NULL)
				meta_val = strdup(mc_metadata->title);
			break;
		case MEDIA_ARTIST:
			if (mc_metadata->artist != NULL)
				meta_val = strdup(mc_metadata->artist);
			break;
		case MEDIA_ALBUM:
			if (mc_metadata->album != NULL)
				meta_val = strdup(mc_metadata->album);
			break;
		case MEDIA_AUTHOR:
			if (mc_metadata->author != NULL)
				meta_val = strdup(mc_metadata->author);
			break;
		case MEDIA_GENRE:
			if (mc_metadata->genre != NULL)
				meta_val = strdup(mc_metadata->genre);
			break;
		case MEDIA_DURATION:
			if (mc_metadata->duration != NULL)
				meta_val = strdup(mc_metadata->duration);
			break;
		case MEDIA_DATE:
			if (mc_metadata->date != NULL)
				meta_val = strdup(mc_metadata->date);
			break;
		case MEDIA_COPYRIGHT:
			if (mc_metadata->copyright != NULL)
				meta_val = strdup(mc_metadata->copyright);
			break;
		case MEDIA_DESCRIPTION:
			if (mc_metadata->description != NULL)
				meta_val = strdup(mc_metadata->description);
			break;
		case MEDIA_TRACK_NUM:
			if (mc_metadata->track_num != NULL)
				meta_val = strdup(mc_metadata->track_num);
			break;
		case MEDIA_PICTURE:
			if (mc_metadata->picture != NULL)
				meta_val = strdup(mc_metadata->picture);
			break;
		default:
			mc_error("Invalid Parameter [%d]", attribute);
			return MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER;
	}

	*value = meta_val;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_client_destroy_playback(mc_playback_h playback)
{
	media_controller_playback_s *mc_playback = (media_controller_playback_s *)playback;

	mc_retvm_if(mc_playback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	MC_SAFE_FREE(mc_playback);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_client_destroy_metadata(mc_metadata_h metadata)
{
	media_controller_metadata_s *mc_metadata = (media_controller_metadata_s *)metadata;

	mc_retvm_if(mc_metadata == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	MC_SAFE_FREE(mc_metadata->title);
	MC_SAFE_FREE(mc_metadata->artist);
	MC_SAFE_FREE(mc_metadata->album);
	MC_SAFE_FREE(mc_metadata->author);
	MC_SAFE_FREE(mc_metadata->genre);
	MC_SAFE_FREE(mc_metadata->duration);
	MC_SAFE_FREE(mc_metadata->date);
	MC_SAFE_FREE(mc_metadata->copyright);
	MC_SAFE_FREE(mc_metadata->description);
	MC_SAFE_FREE(mc_metadata->track_num);
	MC_SAFE_FREE(mc_metadata->picture);

	MC_SAFE_FREE(mc_metadata);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_client_get_latest_server_info(mc_client_h client, char **server_name, mc_server_state_e *server_state)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *latest_server_name = NULL;
	mc_server_state_e latest_server_state = MC_SERVER_STATE_NONE;

	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");
	mc_retvm_if(server_state == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_state is NULL");

	ret = mc_db_get_latest_server_name(mc_client->db_handle, &latest_server_name);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail mc_db_get_latest_server_name [%d]", ret);

	if (latest_server_name != NULL) {
		ret = mc_db_get_server_state(mc_client->db_handle, latest_server_name, &latest_server_state);
		if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
			mc_error("error in getting latest server state %d", ret);
			MC_SAFE_FREE(latest_server_name);
			return ret;
		}

		*server_name = latest_server_name;
		*server_state = latest_server_state;
	} else {
		*server_name = NULL;
		*server_state = MC_SERVER_STATE_NONE;
	}

	return ret;
}

int mc_client_get_server_playback_info(mc_client_h client, const char *server_name, mc_playback_h *playback)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");
	mc_retvm_if(playback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "playback Handle is NULL");

	ret = mc_db_get_playback_info(mc_client->db_handle, server_name, playback);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail mc_db_get_playback_info [%d]", ret);

	return ret;
}

int mc_client_get_server_metadata(mc_client_h client, const char *server_name, mc_metadata_h *metadata)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");
	mc_retvm_if(metadata == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "metadata Handle is NULL");

	ret = mc_db_get_metadata_info(mc_client->db_handle, server_name, metadata);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail mc_db_get_metadata_info [%d]", ret);

	return ret;
}

int mc_client_get_server_shuffle_mode(mc_client_h client, const char *server_name, mc_shuffle_mode_e *mode)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");
	mc_retvm_if(mode == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "mode is NULL");

	ret = mc_db_get_shuffle_mode(mc_client->db_handle, server_name, mode);

	return ret;
}

int mc_client_get_server_repeat_mode(mc_client_h client, const char *server_name, mc_repeat_mode_e *mode)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");
	mc_retvm_if(mode == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "mode is NULL");

	ret = mc_db_get_repeat_mode(mc_client->db_handle, server_name, mode);

	return ret;
}

int mc_client_foreach_server(mc_client_h client, mc_activated_server_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(callback == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "callback is NULL");

	ret = mc_db_foreach_server_list(mc_client->db_handle, callback, user_data);

	return ret;
}

int mc_client_send_playback_state_command(mc_client_h client, const char *server_name, mc_playback_states_e state)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *message = NULL;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");
	mc_retvm_if(((state < MEDIA_PLAYBACK_STATE_PLAYING) || (state > MEDIA_PLAYBACK_STATE_REWIND)), MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "state is invalid");

	message = g_strdup_printf("%s%s%d", mc_client->client_name, MC_STRING_DELIMITER, state);
	mc_retvm_if(message == NULL, MEDIA_CONTROLLER_ERROR_INVALID_OPERATION, "fail making message [%d]", ret);

	ret = mc_util_set_command_availabe(mc_client->client_name, MC_COMMAND_PLAYBACKSTATE, NULL);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error mc_util_set_command_availabe [%d]", ret);
		return ret;
	}

	ret = mc_ipc_send_message(mc_client->dconn, NULL, mc_util_get_interface_name(NULL, MC_SERVER, server_name), MC_DBUS_SIGNAL_NAME_PLAYBACK_STATE_CMD, message, 0);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error mc_ipc_send_message [%d]", ret);
	}

	MC_SAFE_FREE(message);

	return ret;
}

int mc_client_send_custom_command(mc_client_h client, const char *server_name, const char *command, bundle *data, mc_command_reply_received_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *message = NULL;
	int size_r = 0;
	bundle_raw *raw_data = NULL;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	ret = mc_util_set_command_availabe(mc_client->client_name, MC_COMMAND_CUSTOM, command);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error mc_util_set_command_availabe [%d]", ret);
		return ret;
	}

	if(callback) {
		mc_client->reply_cb.callback = callback;
		mc_client->reply_cb.user_data = user_data;
		mc_ipc_register_listener(mc_client->listeners, mc_client->dconn, mc_util_get_interface_name(NULL, MC_CLIENT, mc_client->client_name),
		                         MC_DBUS_SIGNAL_NAME_CMD_REPLY, __client_reply_cb, (void *)&(mc_client->reply_cb));
	}

	if (data) {
		ret = bundle_encode(data, &raw_data, &size_r);
		mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "fail while encoding bundle [%d]", ret);
	}

	if ((size_r == 0)  || (raw_data == NULL)) {
		message = g_strdup_printf("%s%s%s%s%d", mc_client->client_name, MC_STRING_DELIMITER, command, MC_STRING_DELIMITER, size_r);
	} else {
		message = g_strdup_printf("%s%s%s%s%d%s%s", mc_client->client_name, MC_STRING_DELIMITER, command, MC_STRING_DELIMITER, size_r, MC_STRING_DELIMITER, (unsigned char *)raw_data);
	}
	if (message == NULL) {
		mc_error("Error when making message");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = mc_ipc_send_message(mc_client->dconn, NULL, mc_util_get_interface_name(NULL, MC_SERVER, server_name), MC_DBUS_SIGNAL_NAME_CUSTOM_CMD, message, 0);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE)
		mc_error("Error mc_ipc_send_message [%d]", ret);

	MC_SAFE_FREE(message);

	return ret;
}

int mc_client_destroy(mc_client_h client)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_client_s *mc_client = (media_controller_client_s *)client;

	mc_retvm_if(mc_client == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = mc_ipc_unregister_all_listener(mc_client->listeners, mc_client->dconn);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("Error mc_ipc_unregister_all_listener [%d]", ret);
	}

	g_list_free(mc_client->listeners);

	ret = __mc_client_destroy(mc_client);

	return ret;
}
