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

#ifndef __TIZEN_MEDIA_CONTROLLER_PRIVATE_H__
#define __TIZEN_MEDIA_CONTROLLER_PRIVATE_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <glib.h>
#include <dlog.h>
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>
#include "media_controller_server.h"
#include "media_controller_client.h"
#include "media_controller_socket.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "CAPI_MEDIA_CONTROLLER"

#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#define mc_debug(fmt, arg...) do { \
		LOGD(FONT_COLOR_RESET""fmt"", ##arg);     \
	} while (0)

#define mc_info(fmt, arg...) do { \
		LOGI(FONT_COLOR_GREEN""fmt"", ##arg);     \
	} while (0)

#define mc_error(fmt, arg...) do { \
		LOGE(FONT_COLOR_RED""fmt"", ##arg);     \
	} while (0)

#define mc_debug_fenter() do { \
		LOGD(FONT_COLOR_RESET"<Enter>");     \
	} while (0)

#define mc_debug_fleave() do { \
		LOGD(FONT_COLOR_RESET"<Leave>");     \
	} while (0)

#define mc_retm_if(expr, fmt, arg...) do { \
		if (expr) { \
			LOGE(FONT_COLOR_RED""fmt"", ##arg);     \
			return; \
		} \
	} while (0)

#define mc_retvm_if(expr, val, fmt, arg...) do { \
		if (expr) { \
			LOGE(FONT_COLOR_RED""fmt"", ##arg);     \
			return (val); \
		} \
	} while (0)

#define ERR_BUF_LENGHT 256
#define mc_stderror(fmt) do { \
		char mc_stderror_buf[ERR_BUF_LENGHT] = {0, }; \
		strerror_r(errno, mc_stderror_buf, ERR_BUF_LENGHT); \
		LOGE(fmt" : standard error= [%s]", mc_stderror_buf); \
	} while (0)

#define MC_SAFE_FREE(src)			{ if (src) {free(src); src = NULL; } }
#define MC_STRING_VALID(str)				((str != NULL && strlen(str) > 0) ? TRUE : FALSE)
#define MC_STRING_DELIMITER			"VAL_SEP"

/**
* @ingroup CAPI_MEDIA_CONTROLLER_MODULE
* @brief DBus path for media controller.
* @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
*/
#define MC_DBUS_PATH "/org/tizen/mediacontroller/dbus/notify"

/**
* @ingroup CAPI_MEDIA_CONTROLLER_MODULE
* @brief DBus interface to update from server to client.
* @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
*/
#define MC_DBUS_UPDATE_INTERFACE "org.tizen.mediacontroller.update"

/**
* @ingroup CAPI_MEDIA_CONTROLLER_MODULE
* @brief DBus interface prefix name.
* @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
*/
#define MC_DBUS_INTERFACE_PREFIX "org.tizen"

/**
* @ingroup CAPI_MEDIA_CONTROLLER_MODULE
* @brief DBus interface type name.
* @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
*/
#define MC_CLIENT "mediacontroller.client"
#define MC_SERVER "mediacontroller.server"

#define MC_DBUS_SIGNAL_NAME_SERVER_STATE			"server_state"
#define MC_DBUS_SIGNAL_NAME_PLAY_BACK				"playback"
#define MC_DBUS_SIGNAL_NAME_METADATA				"metadata"
#define MC_DBUS_SIGNAL_NAME_PLAYBACK_SHUFFLE		"playback_shuffle"
#define MC_DBUS_SIGNAL_NAME_PLAYBACK_REPEAT			"playback_repeat"
#define MC_DBUS_SIGNAL_NAME_PLAYBACK_STATE_CMD		"playback_state_command"
#define MC_DBUS_SIGNAL_NAME_CUSTOM_CMD				"custom_command"
#define MC_DBUS_SIGNAL_NAME_CMD_REPLY				"command_reply"

#define MC_COMMAND_PLAYBACKSTATE		"_playback_command_"
#define MC_COMMAND_CUSTOM				"_custom_command_"

#define DEFAULT_USER_UID 5001	/* owner */

#define MC_MILLISEC_SLEEP(msec) \
	do { \
		if (msec) { \
			unsigned long sec_t = 0; \
			unsigned long nsec_t = 0; \
			unsigned long cal_time = msec * 1000000; \
			sec_t = cal_time / 1000000000; \
			nsec_t = cal_time % 1000000000; \
			struct timespec reqtime; \
			reqtime.tv_sec = sec_t; \
			reqtime.tv_nsec = nsec_t; \
			nanosleep(&reqtime, NULL); \
		} \
	} while (0)

typedef struct {
	void *callback;
	void *user_data;
	GList *filter_list;
} media_controller_receiver_s;

typedef struct {
	mc_playback_states_e state;
	unsigned long long position;
} media_controller_playback_s;

typedef struct {
	char *title;
	char *artist;
	char *album;
	char *author;
	char *genre;
	char *duration;
	char *date;
	char *copyright;
	char *description;
	char *track_num;
	char *picture;
} media_controller_metadata_s;

typedef struct {
	char *server_name;
	void* db_handle;

	GDBusConnection* dconn;
	int dref_count;

	GList *listeners;

	mc_server_state_e state;
	media_controller_playback_s playback;
	media_controller_metadata_s *metadata;

	media_controller_receiver_s playback_state_reciever;
	media_controller_receiver_s custom_cmd_reciever;
} media_controller_server_s;

typedef struct {
	char		*client_name;
	void		*db_handle;

	GDBusConnection	*dconn;
	int		dref_count;

	GList *listeners;

	media_controller_receiver_s playback_cb;
	media_controller_receiver_s metadata_cb;
	media_controller_receiver_s server_state_cb;
	media_controller_receiver_s shuffle_cb;
	media_controller_receiver_s repeat_cb;
	media_controller_receiver_s reply_cb;
} media_controller_client_s;

/* formal callback to receive signal */
typedef void(*mc_signal_received_cb)(const char *interface_name, const char *signal_name, const char *message, int size, void *user_data);
typedef struct {
	GDBusConnection			*dbus_conn;
	char					*interface_name;
	char					*signal_name;
	mc_signal_received_cb	callback;
	char					*user_data;
	guint					handler;
	char					*key;
} mc_ipc_listener_s;


/* util */
int mc_util_get_own_name(char **name);
char* mc_util_get_interface_name(const char *type, const char *name);
int mc_util_make_filter_interface_name(const char *prefix, const char *filter, char **interface_name);
int mc_util_set_command_availabe(const char *name, const char *command_type, const char *command);
int mc_util_get_command_availabe(const char *name, const char *command_type, const char *command);

/* for d-bus IPC */
int mc_ipc_get_dbus_connection(GDBusConnection **conn, int *dref_count);
int mc_ipc_unref_dbus_connection(GDBusConnection *conn, int *dref_count);
int mc_ipc_register_listener(GList *manage_list, GDBusConnection *connection, const char *interface_name, const char *signal_name, mc_signal_received_cb callback, void *user_data);
int mc_ipc_unregister_listener(GList *manage_list, GDBusConnection *connection, const char *interface_name, const char *signal_name);
int mc_ipc_unregister_all_listener(GList *manage_list, GDBusConnection *connection);
int mc_ipc_send_message(GDBusConnection *connection, const char *dbus_name, const char *interface_name, const char* signal_name, const char* message, int flags);
int mc_ipc_send_message_to_server(mc_msg_type_e msg_type, const char *request_msg);
int mc_ipc_service_connect(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_MEDIA_CONTROLLER_PRIVATE_H__ */
