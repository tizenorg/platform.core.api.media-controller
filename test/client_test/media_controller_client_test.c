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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gprintf.h>

#include <media_controller_client.h>
#include "media_controller_private.h"


#define PACKAGE "media_controller_client_test"

/*===========================================================================================
|                                             |
|  LOCAL DEFINITIONS AND DECLARATIONS FOR MODULE                      |
|                                               |
========================================================================================== */
/*---------------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS:                     |
---------------------------------------------------------------------------*/

static mc_client_h g_mc_client;
static char* g_server_name;

static mc_playback_states_e g_playback_state;

GMainLoop *mainloop = NULL;

#define MAX_STRING_LEN    2048

/*---------------------------------------------------------------------------
|    LOCAL CONSTANT DEFINITIONS:                      |
---------------------------------------------------------------------------*/
#define DEFAULT_SERVICE "com.samsung.mcontroller_service"

enum {
	CURRENT_STATE_MAIN_MENU,
	CURRENT_STATE_INFORMATION_GET_MENU,
};

enum {
	CURRENT_STATE_INFORMATION_GET_MODE,
	CURRENT_STATE_INFORMATION_GET_METADATA,
};

/*---------------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS:                      |
---------------------------------------------------------------------------*/
int g_menu_state = CURRENT_STATE_MAIN_MENU;
int g_menu_information_state = CURRENT_STATE_INFORMATION_GET_MODE;

/*---------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:                       |
---------------------------------------------------------------------------*/

void _mc_server_state_updated_cb(const char *server_name, mc_server_state_e state, void *user_data)
{
	media_controller_client_s* _client = (media_controller_client_s*)g_mc_client;
	mc_debug("[Client:%s] Server state(%d) updated from server[%s]", _client->client_name, state, server_name);
}

void _mc_playback_updated_cb(const char *server_name, mc_playback_h playback, void *user_data)
{
	media_controller_client_s* _client = (media_controller_client_s*)g_mc_client;
	unsigned long long position = 0;
	mc_playback_states_e playback_state = MEDIA_PLAYBACK_STATE_NONE;

	mc_client_get_playback_position(playback, &position);
	mc_client_get_playback_state(playback, &playback_state);

	mc_debug("[Client:%s] Playback updated from server[%s] playback_state[%d] position[%lld]", _client->client_name, server_name, playback_state, position);
}

void _mc_metadata_updated_cb(const char *server_name, mc_metadata_h metadata, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	int idx = 0;
	char *str_val = NULL;
	media_controller_client_s* _client = (media_controller_client_s*)g_mc_client;
	mc_debug("[Client:%s] Metadata updated from server[%s] ", _client->client_name, server_name);

	for(idx = 0; idx <= MEDIA_PICTURE; idx++)
	{
		ret = mc_client_get_metadata(metadata, idx, &str_val);
		if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
			mc_error ("Fail to mc_client_get_metadata");
		mc_debug ("metadata [%d] val [%s]\n", idx, str_val);

		if(str_val)
			g_free(str_val);
	}
}

void _mc_shuffle_mode_updated_cb(const char *server_name, mc_shuffle_mode_e mode, void *user_data)
{
	media_controller_client_s* _client = (media_controller_client_s*)g_mc_client;
	mc_debug("[Client:%s] Shuffle mode(%d) updated from server[%s]", _client->client_name, mode, server_name);
}

void _mc_repeat_mode_updated_cb(const char *server_name, mc_repeat_mode_e mode, void *user_data)
{
	media_controller_client_s* _client = (media_controller_client_s*)g_mc_client;
	mc_debug("[Client:%s] Repeat mode(%d) updated from server[%s]", _client->client_name, mode, server_name);
}

bool _mc_activated_server_cb(const char *server_name, void *user_data)
{
	media_controller_client_s* _client = (media_controller_client_s*)g_mc_client;
	mc_debug("[Client:%s] Activated server_name: %s", _client->client_name, server_name);
	return TRUE;
}

void _mc_command_reply_received_cb (const char *server_name, int result_code, bundle *data, void *user_data)
{
	char* value = NULL;
	media_controller_client_s* _client = (media_controller_client_s*)g_mc_client;

	if(data)
		bundle_get_str(data, "key1", &value);

	mc_debug("[Client:%s] Command Reply (key1: %s)received from server[%s] result code[%d]", _client->client_name, value, server_name, result_code);
}

static gboolean _create ()
{
	g_print("== create \n");
	int ret;

	ret = mc_client_create(&g_mc_client);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to create media contoller client");
		return FALSE;
	}
	g_print ("== success create \n");

	return TRUE;
}

static gboolean _foreach ()
{
	g_print("== create \n");
	int ret;

	ret = mc_client_foreach_server(g_mc_client, _mc_activated_server_cb, NULL);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to foreach media contoller client");
		return FALSE;
	}
	g_print ("== success create \n");

	return TRUE;
}

static gboolean _set ()
{
	g_print ("== set default callback \n");
	int ret = TRUE;

	ret = mc_client_set_server_update_cb(g_mc_client, _mc_server_state_updated_cb, NULL);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to set param and initialize %d", ret);
		return FALSE;
	}

	ret = mc_client_set_playback_update_cb(g_mc_client, _mc_playback_updated_cb, NULL);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to set param and initialize %d", ret);
		return FALSE;
	}

	ret = mc_client_set_metadata_update_cb(g_mc_client, _mc_metadata_updated_cb, NULL);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to set param and initialize %d", ret);
		return FALSE;
	}

	ret = mc_client_set_shuffle_mode_update_cb(g_mc_client, _mc_shuffle_mode_updated_cb, NULL);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to set param and initialize %d", ret);
		return FALSE;
	}

	ret = mc_client_set_repeat_mode_update_cb(g_mc_client, _mc_repeat_mode_updated_cb, NULL);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to set param and initialize %d", ret);
		return FALSE;
	}

	g_print("==\n");

	return ret;
}

static gboolean _unset()
{
	g_print ("== unset callback \n");
	int ret;

	ret = mc_client_unset_server_update_cb(g_mc_client);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to deinitialize %d", ret);
		return FALSE;
	}

	ret = mc_client_unset_playback_update_cb(g_mc_client);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to deinitialize %d", ret);
		return FALSE;
	}

	ret = mc_client_unset_metadata_update_cb(g_mc_client);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to deinitialize %d", ret);
		return FALSE;
	}

	ret = mc_client_unset_shuffle_mode_update_cb(g_mc_client);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to deinitialize %d", ret);
		return FALSE;
	}

	ret = mc_client_unset_repeat_mode_update_cb(g_mc_client);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to deinitialize %d", ret);
		return FALSE;
	}

	g_print ("== success destroy \n");

	return TRUE;
}

static gboolean _get_info(int type)
{
	g_print ("== get information \n");
	int ret;
	mc_server_state_e server_state;
	mc_playback_h playback;
	mc_playback_states_e playback_state;
	unsigned long long playback_position;
	mc_metadata_h metadata;
	char* metadata_value;
	mc_shuffle_mode_e shuffle_mode;
	mc_repeat_mode_e repeate_mode;

	switch(type)
	{
		case 1:
			ret = mc_client_get_latest_server_info(g_mc_client, &g_server_name, &server_state);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to get latest server info");
				return FALSE;
			}
			g_print ("get server name: %s, state: %d", g_server_name, server_state);
			break;
		case 2:
			ret = mc_client_get_server_playback_info(g_mc_client, g_server_name, &playback);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to get playback info");
				return FALSE;
			}
			ret = mc_client_get_playback_state(playback, &playback_state);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to get playback state");
			}
			ret = mc_client_get_playback_position(playback, &playback_position);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to get playback position");
			}
			g_print ("playback state: %d, position: %lld", playback_state, playback_position);

			ret = mc_client_destroy_playback(playback);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to destroy playback");
				return FALSE;
			}
			break;
		case 3:
			ret = mc_client_get_server_metadata(g_mc_client, g_server_name, &metadata);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to get metadata");
				return FALSE;
			}
			ret = mc_client_get_metadata(metadata, MEDIA_TITLE, &metadata_value);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to get infot");
			}
			g_print ("metadata title: %s", metadata_value);

			ret = mc_client_destroy_metadata(metadata);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to destroy metadata");
				return FALSE;
			}
			free(metadata_value);
			break;
		case 4:
			ret = mc_client_get_server_shuffle_mode(g_mc_client, g_server_name, &shuffle_mode);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to get infot");
				return FALSE;
			}
			g_print ("shuffle mode: %d", shuffle_mode);
			break;
		case 5:
			ret = mc_client_get_server_repeat_mode(g_mc_client, g_server_name, &repeate_mode);
			if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print ("Fail to get infot");
				return FALSE;
			}
			g_print ("repeate mode: %d", repeate_mode);
			break;
		default:
			g_print ("== unknown type!\n");
			return TRUE;
	}

	g_print ("== success get information \n");

	return TRUE;
}

static gboolean _send()
{
	g_print ("== send command to latest server \n");
	int ret;

	if (g_playback_state == MEDIA_PLAYBACK_STATE_PLAYING)
		g_playback_state = MEDIA_PLAYBACK_STATE_STOPPED;
	else
		g_playback_state = MEDIA_PLAYBACK_STATE_PLAYING;
	ret = mc_client_send_playback_state_command(g_mc_client, g_server_name, g_playback_state);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to send playback state command %d", ret);
		return FALSE;
	}

	usleep(500000);

	bundle * bundle_data = bundle_create();
	bundle_add_str(bundle_data, "key1", "val1");
	bundle_add_str(bundle_data, "key2", "val2");
	bundle_add_str(bundle_data, "key3", "val3");
	bundle_add_str(bundle_data, "key4", "val4");

	ret = mc_client_send_custom_command(g_mc_client, g_server_name, "Custom Command", bundle_data, _mc_command_reply_received_cb, NULL);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		bundle_free(bundle_data);
		mc_error ("Fail to send custom command %d", ret);
		return FALSE;
	}
	bundle_free(bundle_data);

	g_print ("== success send command \n");

	return TRUE;
}

static gboolean _destroy()
{
	g_print ("== destroy \n");
	int ret;

	ret = mc_client_destroy(g_mc_client);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to destroy media contoller client");
		return FALSE;
	}

	g_print ("== success destroy \n");
	g_mc_client = NULL;
	return TRUE;
}


/***************************************************************/
/**  Testsuite */
/***************************************************************/
void quit (void)
{
	if (g_mc_client != NULL)
		_destroy();

	g_main_loop_quit(mainloop);
}

void reset_current_information_menu_state()
{
	g_menu_information_state = CURRENT_STATE_INFORMATION_GET_MODE;
	return;
}

void reset_current_menu_state()
{
	g_menu_state = CURRENT_STATE_MAIN_MENU;
	return;
}

static void display_information_menu (void)
{
	g_print("\n");
	g_print("====================================================\n");
	g_print("    media controller test(client): Get info menu\n");
	g_print("----------------------------------------------------\n");
	g_print("1. get latest server info \n");
	g_print("2. get latest server playback \n");
	g_print("3. get latest server metadata \n");
	g_print("4. get latest server shuffle mode \n");
	g_print("5. get latest server repeate moder \n");
	g_print("0. back \n");
	g_print("----------------------------------------------------\n");
	g_print("====================================================\n");

}

static void display_main_menu (void)
{
	g_print("\n");
	g_print("====================================================\n");
	g_print("   media controller test(client): Main menu v0.1\n");
	g_print("----------------------------------------------------\n");
	g_print("1. create media controller client \n");
	g_print("2. foreach server list \n");
	g_print("3. set default callback \n");
	g_print("4. unset default callback \n");
	g_print("5. get information from server \n");
	g_print("6. send command \n");
	g_print("9. destroy media controller client \n");
	g_print("0. quit \n");
	g_print("----------------------------------------------------\n");
	g_print("====================================================\n");

}

static void display_menu (void)
{
	if (g_menu_state == CURRENT_STATE_MAIN_MENU) {
		display_main_menu ();
	} else if (g_menu_state == CURRENT_STATE_INFORMATION_GET_MENU) {
		display_information_menu();
	} else {
		g_print("*** Unknown status.\n");
	}
}

void _interpret_information_menu (char *cmd)
{
	int len = strlen(cmd);

	if (len == 1) {
		if ( !strncmp(cmd, "1", len)) {
			_get_info(1);
		} else if ( !strncmp(cmd, "2", len)) {
			_get_info(2);
		} else if ( !strncmp(cmd, "3", len)) {
			_get_info(3);
		} else if ( !strncmp(cmd, "4", len)) {
			_get_info(4);
		} else if ( !strncmp(cmd, "5", len)) {
			_get_info(5);
		} else if ( !strncmp(cmd, "0", len)) {
			reset_current_menu_state();
		}
	} else {
		g_print("wrong command\n");
	}
}

void _interpret_main_menu (char *cmd)
{
	int len = strlen(cmd);

	if (len == 1) {
		if ( !strncmp(cmd, "1", len)) {
			_create();
		} else if ( !strncmp(cmd, "2", len)) {
			_foreach();
		} else if ( !strncmp(cmd, "3", len)) {
			_set();
		} else if ( !strncmp(cmd, "4", len)) {
			_unset();
		} else if ( !strncmp(cmd, "5", len)) {
			g_menu_state = CURRENT_STATE_INFORMATION_GET_MENU;
		} else if ( !strncmp(cmd, "6", len)) {
			_send();
		}  else if ( !strncmp(cmd, "9", len)) {
			_destroy();
		} else if ( !strncmp(cmd, "0", len)) {
			quit();
		}
	} else {
		g_print("wrong command\n");
	}
}

static void interpret_cmd (char *cmd)
{
	switch (g_menu_state) {
		case CURRENT_STATE_MAIN_MENU:
			_interpret_main_menu(cmd);
			display_menu();
			break;
		case CURRENT_STATE_INFORMATION_GET_MENU:
			_interpret_information_menu(cmd);
			display_menu();
			break;
		default:
			g_print("Invalid command\n");
	}
}

gboolean input (GIOChannel *channel)
{
	gchar buf[MAX_STRING_LEN];
	gsize read;
	GError *error = NULL;

	g_io_channel_read_chars(channel, buf, MAX_STRING_LEN, &read, &error);

	buf[read] = '\0';
	g_strstrip(buf);
	interpret_cmd (buf);

	return TRUE;
	
}

int client_sequential_test(void)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *server_name = NULL;
	mc_server_state_e server_state = MC_SERVER_STATE_NONE;
	mc_shuffle_mode_e shuffle_mode = SHUFFLE_MODE_OFF;
	mc_repeat_mode_e repeat_mode = REPEAT_MODE_OFF;
	mc_metadata_h metadata = NULL;
	mc_playback_h playback = NULL;
	char *str_val = NULL;
	int idx = 0;
	mc_playback_states_e playback_state = MEDIA_PLAYBACK_STATE_REWIND;
	unsigned long long playback_position = 0;

	/*Create Client*/
	ret = mc_client_create(&g_mc_client);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to mc_client_create");
		return ret;
	}

	/*Get Latest Server Info*/
	ret = mc_client_get_latest_server_info(g_mc_client, &server_name, &server_state);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to mc_client_create");
		return ret;
	}

	g_print ("latest server name[%s] server_state[%d]\n", server_name, server_state);

	if(server_name == NULL)
		return MEDIA_CONTROLLER_ERROR_NONE;

	/*Get Foreach server*/
	ret = mc_client_foreach_server(g_mc_client, _mc_activated_server_cb, NULL);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
		g_print ("Fail to mc_client_foreach_server\n");

	/*Get Playback Info*/
	ret = mc_client_get_server_playback_info(g_mc_client, server_name, &playback);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
		g_print ("Fail to mc_client_get_server_playback_info\n");

	ret = mc_client_get_playback_state(playback, &playback_state);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
		g_print ("Fail to mc_client_get_playback_state\n");

	ret = mc_client_get_playback_position(playback, &playback_position);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
		g_print ("Fail to mc_client_get_server_playback_info\n");

	g_print ("playback_state[%d] playback_position[%lld]\n", playback_state, playback_position);

	ret = mc_client_destroy_playback(playback);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
		g_print ("Fail to mc_client_destroy_playback\n");

	/*Get Metadata*/
	ret = mc_client_get_server_metadata(g_mc_client, server_name, &metadata);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
		g_print ("Fail to mc_client_get_server_metadata\n");

	for(idx = 0; idx <= MEDIA_PICTURE; idx++)
	{
		ret = mc_client_get_metadata(metadata, idx, &str_val);
		if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
			g_print ("Fail to mc_client_get_metadata\n");
		g_print ("metadata [%d] val [%s]\n", idx, str_val);
		if(str_val)
			g_free(str_val);
	}

	ret = mc_client_destroy_metadata(metadata);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
		g_print ("Fail to mc_client_destroy_metadata\n");

	/*Get Shuffle / Repeat mode*/
	ret = mc_client_get_server_shuffle_mode(g_mc_client, server_name, &shuffle_mode);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
		g_print ("Fail to mc_client_get_server_shuffle_mode\n");

	ret = mc_client_get_server_repeat_mode(g_mc_client, server_name, &repeat_mode);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE)
		g_print ("Fail to mc_client_get_server_repeat_mode\n");

	g_print ("shuffle_mode[%d] repeat_mode[%d]\n", shuffle_mode, repeat_mode);

	g_free(server_name);

	ret = mc_client_destroy(g_mc_client);
	if ( ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print ("Fail to mc_client_create\n");
		return ret;
	}

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int main(int argc, char **argv)
{
	GIOChannel *stdin_channel = NULL;
	stdin_channel = g_io_channel_unix_new(0);
	g_io_channel_set_flags(stdin_channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch (stdin_channel, G_IO_IN, (GIOFunc)input, NULL);

	g_playback_state = MEDIA_PLAYBACK_STATE_PLAYING;

	mainloop = g_main_loop_new(NULL, FALSE);

	display_menu();

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	return 0;
}
