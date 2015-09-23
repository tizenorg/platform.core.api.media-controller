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

static mc_client_h g_mc_server;
static int g_metadata_type;
static mc_shuffle_mode_e g_shuffle_mode;
static mc_repeat_mode_e g_repeat_mode;

GMainLoop *mainloop = NULL;

#define MAX_STRING_LEN    2048

/*---------------------------------------------------------------------------
|    LOCAL CONSTANT DEFINITIONS:                      |
---------------------------------------------------------------------------*/
/*#define DEFAULT_SERVICE "com.samsung.mcontroller_service" */

enum {
	CURRENT_STATE_MAIN_MENU,
	CURRENT_STATE_INFORMATION_SET_MENU,
	CURRENT_STATE_INFORMATION_UPDATE_MENU,
};

enum {
	CURRENT_STATE_SET_MODE_NONE,
	CURRENT_STATE_SET_PLAYBACK_STATE,
	CURRENT_STATE_SET_PLAYBACK_POSITION,
	CURRENT_STATE_SET_METADATA_NAME,
	CURRENT_STATE_SET_METADATA_VALUE,
};

/*---------------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS:                      |
---------------------------------------------------------------------------*/
int g_menu_state = CURRENT_STATE_MAIN_MENU;
int g_menu_set_state = CURRENT_STATE_SET_MODE_NONE;

/*---------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:                       |
---------------------------------------------------------------------------*/

void __playback_state_command_received_cb(const char *client_name, mc_playback_states_e state, void *user_data)
{
	media_controller_server_s *mc_server = (media_controller_server_s *)g_mc_server;
	mc_debug("[%s] recieved playback state:[%d] from [%s]", mc_server->server_name, state, client_name);
}

void __custom_command_received_cb(const char *client_name, const char *command, bundle *data, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	media_controller_server_s *mc_server = (media_controller_server_s *)g_mc_server;

	char *bundle_data = NULL;
	char *get_value1 = NULL;
	char *get_value2 = NULL;
	char *get_value3 = NULL;
	char *get_value4 = NULL;

	if (data) {
		bundle_get_str(data, "key1", &get_value1);
		bundle_get_str(data, "key2", &get_value2);
		bundle_get_str(data, "key3", &get_value3);
		bundle_get_str(data, "key4", &get_value4);

		bundle_data = g_strdup_printf("%s, %s, %s, %s", get_value1, get_value2, get_value3, get_value4);
	}

	mc_debug("[%s] recieved command:[%s] from [%s]", mc_server->server_name, command, client_name);
	mc_debug("[%s] recieved bundle:[%s] from [%s]", mc_server->server_name, bundle_data, client_name);

	bundle *bundle_reply = bundle_create();
	bundle_add_str(bundle_reply, "key1", "result1");
	bundle_add_str(bundle_reply, "key2", "result2");
	bundle_add_str(bundle_reply, "key3", "result3");
	bundle_add_str(bundle_reply, "key4", "result4");

	ret = mc_server_send_command_reply(g_mc_server, client_name, 0, bundle_reply);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE)
		mc_error("Fail to mc_server_send_command_reply");

	bundle_free(bundle_reply);
}

static gboolean _create()
{
	g_print("== create \n");
	int ret;

	ret = mc_server_create(&g_mc_server);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to create media contoller server");
		return FALSE;
	}
	g_print("== success create \n");

	return TRUE;
}

static gboolean _set_cb()
{
	g_print("== set default callback for commands \n");
	int ret;

	ret = mc_server_set_playback_state_command_received_cb(g_mc_server, __playback_state_command_received_cb, NULL);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to create media contoller server");
		return FALSE;
	}

	ret = mc_server_set_custom_command_received_cb(g_mc_server, __custom_command_received_cb, NULL);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to create media contoller server");
		return FALSE;
	}

	g_print("== success set default callback \n");

	return TRUE;
}

static gboolean _set_info(int type, char *cmd)
{
	g_print("== get information \n");
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	int playback_state = 0;
	unsigned long long playback_position;
	char *metadata = NULL;

	switch (type) {
		case CURRENT_STATE_SET_PLAYBACK_STATE:
			playback_state = atoi(cmd);
			ret = mc_server_set_playback_state(g_mc_server, playback_state);
			if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print("Fail to set playback state");
				return FALSE;
			}
			g_print("set state: %d", playback_state);
			break;
		case CURRENT_STATE_SET_PLAYBACK_POSITION:
			playback_position = atoi(cmd);
			ret = mc_server_set_playback_position(g_mc_server, playback_position);
			if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print("Fail to set playback position");
				return FALSE;
			}
			g_print("set position: %lld", playback_position);
			break;
		case CURRENT_STATE_SET_METADATA_NAME:
			g_metadata_type = atoi(cmd);
			g_print("set metadata name: %d", g_metadata_type);
			break;
		case CURRENT_STATE_SET_METADATA_VALUE:
			metadata = strdup(cmd);
			ret = mc_server_set_metadata(g_mc_server, g_metadata_type, metadata);
			if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print("Fail to set metadata");
				if(metadata != NULL)
					free(metadata);
				return FALSE;
			}
			g_print("set metadata value: %s", metadata);
			if(metadata != NULL)
				free(metadata);
			break;
		default:
			g_print(" == unknown type!\n");
			return TRUE;
	}

	g_print(" == success get information \n");

	return TRUE;
}

static gboolean _update_info(int type)
{
	g_print(" == update information \n");
	int ret;

	switch (type) {
		case 1:
			ret = mc_server_update_playback_info(g_mc_server);
			if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print("Fail to update playback info err=%d", ret);
				return FALSE;
			}
			break;
		case 2:
			ret = mc_server_update_metadata(g_mc_server);
			if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print("Fail to update metadata err=%d", ret);
				return FALSE;
			}
			break;
		case 3:
			if (g_shuffle_mode == MC_SHUFFLE_MODE_OFF)
				g_shuffle_mode = MC_SHUFFLE_MODE_ON;
			else
				g_shuffle_mode = MC_SHUFFLE_MODE_OFF;
			ret = mc_server_update_shuffle_mode(g_mc_server, g_shuffle_mode);
			if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print("Fail to update shuffle mode err=%d", ret);
				return FALSE;
			}
			break;
		case 4:
			if (g_repeat_mode == MC_REPEAT_MODE_OFF)
				g_repeat_mode = MC_REPEAT_MODE_ON;
			else
				g_repeat_mode = MC_REPEAT_MODE_OFF;
			ret = mc_server_update_repeat_mode(g_mc_server, g_repeat_mode);
			if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
				g_print("Fail to update repeat mode err=%d", ret);
				return FALSE;
			}
			break;
		default:
			g_print(" == unknown type!\n");
			return TRUE;
	}

	g_print(" == success update information \n");

	return TRUE;
}

static gboolean _destroy()
{
	g_print("== destroy \n");
	int ret;

	ret = mc_server_destroy(g_mc_server);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to destroy media contoller server");
		return FALSE;
	}

	g_print("== success destroy \n");
	g_mc_server = NULL;
	return TRUE;
}


/***************************************************************/
/**  Testsuite */
/***************************************************************/
void quit(void)
{
	if (g_mc_server != NULL)
		_destroy();

	g_main_loop_quit(mainloop);
}

void reset_current_set_menu_state()
{
	g_menu_set_state = CURRENT_STATE_SET_MODE_NONE;
	return;
}

void reset_current_menu_state()
{
	g_menu_state = CURRENT_STATE_MAIN_MENU;
	return;
}

static void display_update_info_menu(void)
{
	g_print("\n");
	g_print("====================================================\n");
	g_print("    media controller test(server): Update info menu\n");
	g_print("----------------------------------------------------\n");
	g_print("1. update server playback \n");
	g_print("2. update server metadata \n");
	g_print("3. update server shuffle mode \n");
	g_print("4. update server repeate moder \n");
	g_print("0. back \n");
	g_print("----------------------------------------------------\n");
	g_print("====================================================\n");

}

static void display_set_info_menu(void)
{
	g_print("\n");
	g_print("====================================================\n");
	g_print("    media controller test(server): Set info menu\n");
	g_print("----------------------------------------------------\n");
	g_print("1. set server playback state\n");
	g_print("2. set server playback position \n");
	g_print("3. set server metadata name\n");
	g_print("4. set server metadata value\n");
	g_print("0. back \n");
	g_print("----------------------------------------------------\n");
	g_print("====================================================\n");

}

static void display_main_menu(void)
{
	g_print("\n");
	g_print("====================================================\n");
	g_print("   media controller test(server): Main menu v0.1\n");
	g_print("----------------------------------------------------\n");
	g_print("1. create media controller server \n");
	g_print("2. set default command callback \n");
	g_print("3. set information to client \n");
	g_print("4. update information to client \n");
	g_print("9. destroy media controller server \n");
	g_print("0. quit \n");
	g_print("----------------------------------------------------\n");
	g_print("====================================================\n");

}

static void display_menu(void)
{
	if (g_menu_state == CURRENT_STATE_MAIN_MENU) {
		display_main_menu();
	} else if (g_menu_state == CURRENT_STATE_INFORMATION_SET_MENU) {
		display_set_info_menu();
	} else if (g_menu_state == CURRENT_STATE_INFORMATION_UPDATE_MENU) {
		display_update_info_menu();
	} else {
		g_print("*** Unknown status.\n");
	}
}

void _interpret_update_info_menu(char *cmd)
{
	int len = strlen(cmd);

	if (len == 1) {
		if (!strncmp(cmd, "1", len)) {
			_update_info(1);
		} else if (!strncmp(cmd, "2", len)) {
			_update_info(2);
		} else if (!strncmp(cmd, "3", len)) {
			_update_info(3);
		} else if (!strncmp(cmd, "4", len)) {
			_update_info(4);
		} else if (!strncmp(cmd, "0", len)) {
			reset_current_menu_state();
		}
	} else {
		g_print("wrong command\n");
	}
}

void _interpret_set_info_menu(char *cmd)
{
	int len = strlen(cmd);

	if (len == 1) {
		if (!strncmp(cmd, "1", len)) {
			g_menu_set_state = CURRENT_STATE_SET_PLAYBACK_STATE;
		} else if (!strncmp(cmd, "2", len)) {
			g_menu_set_state = CURRENT_STATE_SET_PLAYBACK_POSITION;
		} else if (!strncmp(cmd, "3", len)) {
			g_menu_set_state = CURRENT_STATE_SET_METADATA_NAME;
		} else if (!strncmp(cmd, "4", len)) {
			g_menu_set_state = CURRENT_STATE_SET_METADATA_VALUE;
		} else if (!strncmp(cmd, "0", len)) {
			reset_current_menu_state();
			display_menu();
		}
	} else {
		g_print("wrong command\n");
	}
}

void _interpret_main_menu(char *cmd)
{
	int len = strlen(cmd);

	if (len == 1) {
		if (!strncmp(cmd, "1", len)) {
			_create();
		} else if (!strncmp(cmd, "2", len)) {
			_set_cb();
		} else if (!strncmp(cmd, "3", len)) {
			g_menu_state = CURRENT_STATE_INFORMATION_SET_MENU;
		} else if (!strncmp(cmd, "4", len)) {
			g_menu_state = CURRENT_STATE_INFORMATION_UPDATE_MENU;
		}   else if (!strncmp(cmd, "9", len)) {
			_destroy();
		} else if (!strncmp(cmd, "0", len)) {
			quit();
		}
	} else {
		g_print("wrong command\n");
	}
}

static void interpret_cmd(char *cmd)
{
	switch (g_menu_state) {
		case CURRENT_STATE_MAIN_MENU:
			_interpret_main_menu(cmd);
			display_menu();
			break;
		case CURRENT_STATE_INFORMATION_SET_MENU:
			switch (g_menu_set_state) {
				case CURRENT_STATE_SET_MODE_NONE:
					_interpret_set_info_menu(cmd);
					break;
				case CURRENT_STATE_SET_PLAYBACK_STATE:
				case CURRENT_STATE_SET_PLAYBACK_POSITION:
				case CURRENT_STATE_SET_METADATA_NAME:
				case CURRENT_STATE_SET_METADATA_VALUE:
					_set_info(g_menu_set_state, cmd);
					reset_current_set_menu_state();
					display_menu();
				default:
					break;
			}
			break;
		case CURRENT_STATE_INFORMATION_UPDATE_MENU:
			_interpret_update_info_menu(cmd);
			display_menu();
			break;
		default:
			g_print("Invalid command\n");
	}
}

gboolean input(GIOChannel *channel)
{
	gchar buf[MAX_STRING_LEN];
	gsize read;
	GError *error = NULL;

	g_io_channel_read_chars(channel, buf, MAX_STRING_LEN, &read, &error);

	buf[read] = '\0';
	g_strstrip(buf);
	interpret_cmd(buf);

	return TRUE;

}

int server_sequential_test(void)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;

	ret = mc_server_create(&g_mc_server);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to mc_server_create\n");
		return ret;
	}

	ret = mc_server_set_playback_state(g_mc_server, MC_PLAYBACK_STATE_PLAYING);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to mc_server_set_playback_state\n");
		return ret;
	}

	ret = mc_server_set_playback_position(g_mc_server, 10000);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to mc_server_set_playback_state\n");
		return ret;
	}

	ret = mc_server_update_playback_info(g_mc_server);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to mc_server_update_playback_info\n");
		/*return ret; */
	}

	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_TITLE, "media_title");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_ARTIST, "media_artist");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_ALBUM, "media_album");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_AUTHOR, "media_author");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_GENRE, "media_genre");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_DURATION, "200");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_DATE, "media_date");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_COPYRIGHT, "media_copyright");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_DESCRIPTION, "media_description");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_TRACK_NUM, "media_track_num 3/10");
	ret = mc_server_set_metadata(g_mc_server, MC_META_MEDIA_PICTURE, "media_picture_path");
	ret = mc_server_update_metadata(g_mc_server);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to mc_server_update_metadata\n");
		/*return ret; */
	}

	ret = mc_server_update_shuffle_mode(g_mc_server, MC_SHUFFLE_MODE_ON);
	ret = mc_server_update_repeat_mode(g_mc_server, MC_REPEAT_MODE_ON);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to mc_server_update_repeat_mode\n");
		return ret;
	}

	ret = mc_server_destroy(g_mc_server);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		g_print("Fail to mc_server_destroy");
		return ret;
	}

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int main(int argc, char **argv)
{
	GIOChannel *stdin_channel;
	stdin_channel = g_io_channel_unix_new(0);
	g_io_channel_set_flags(stdin_channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)input, NULL);

	g_shuffle_mode = MC_SHUFFLE_MODE_OFF;
	g_repeat_mode = MC_REPEAT_MODE_OFF;

	mainloop = g_main_loop_new(NULL, FALSE);

	display_menu();

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	return 0;
}
