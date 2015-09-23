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

#ifndef __TIZEN_MEDIA_CONTROLLER_TYPE_H__
#define __TIZEN_MEDIA_CONTROLLER_TYPE_H__

#include <tizen.h>
#include <bundle.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file media_controller_type.h
 * @brief This file contains API related to media controller enumerations and types. \n
 *        Listed APIs are called when client send and receive event.  \n
 */

/**
* @addtogroup CAPI_MEDIA_CONTROLLER_MODULE
 * @{
 */

/**
 * @brief The structure type for the media server handle.
 * @since_tizen 2.4
 */
typedef void *mc_server_h;

/**
 * @brief The structure type for the media client handle.
 * @since_tizen 2.4
 */
typedef void *mc_client_h;

/**
 * @brief The structure type for the media controller playback handle.
 * @since_tizen 2.4
 */
typedef void *mc_playback_h;

/**
 * @brief The structure type for the media controller metadata handle.
 * @since_tizen 2.4
 */
typedef void *mc_metadata_h;

/**
 * @brief Enumeration for the media controller error.
 * @since_tizen 2.4
 */
typedef enum {
	MEDIA_CONTROLLER_ERROR_NONE						= TIZEN_ERROR_NONE,						/**< Successful */
	MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER		= TIZEN_ERROR_INVALID_PARAMETER,		/**< Invalid parameter */
	MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY			= TIZEN_ERROR_OUT_OF_MEMORY,			/**< Out of memory */
	MEDIA_CONTROLLER_ERROR_INVALID_OPERATION		= TIZEN_ERROR_INVALID_OPERATION,		/**< Invalid Operation */
	MEDIA_CONTROLLER_ERROR_FILE_NO_SPACE_ON_DEVICE		= TIZEN_ERROR_FILE_NO_SPACE_ON_DEVICE,	/**< No space left on device */
	MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED		= TIZEN_ERROR_PERMISSION_DENIED,		/**< Permission denied */
} mc_error_e;

/**
 * @brief Enumeration for the media controller server state.
 * @since_tizen 2.4
 */
typedef enum {
	MC_SERVER_STATE_NONE = 0,					/**< None state*/
	MC_SERVER_STATE_ACTIVATE,					/**< Activate state*/
	MC_SERVER_STATE_DEACTIVATE,					/**< Deactivate state*/
} mc_server_state_e;

/**
 * @brief Enumeration for the media meta info.
 * @since_tizen 2.4
 */
typedef enum {
	MC_META_MEDIA_TITLE = 0,					/**< Title */
	MC_META_MEDIA_ARTIST,						/**< Artist */
	MC_META_MEDIA_ALBUM,						/**< Album */
	MC_META_MEDIA_AUTHOR,						/**< Author */
	MC_META_MEDIA_GENRE,						/**< Genre */
	MC_META_MEDIA_DURATION,						/**< Duration */
	MC_META_MEDIA_DATE,							/**< Date */
	MC_META_MEDIA_COPYRIGHT,					/**< Copyright */
	MC_META_MEDIA_DESCRIPTION,					/**< Description */
	MC_META_MEDIA_TRACK_NUM,					/**< Track Number */
	MC_META_MEDIA_PICTURE,						/**< Picture. Album Art */
} mc_meta_e;

/**
 * @brief Enumeration for the media playback state.
 * @since_tizen 2.4
 */
typedef enum {
	MC_PLAYBACK_STATE_NONE = 0,				/**< None */
	MC_PLAYBACK_STATE_PLAYING,				/**< Play */
	MC_PLAYBACK_STATE_PAUSED,				/**< Pause */
	MC_PLAYBACK_STATE_STOPPED,				/**< Stop */
	MC_PLAYBACK_STATE_NEXT_FILE,			/**< Next file */
	MC_PLAYBACK_STATE_PREV_FILE,			/**< Previous file */
	MC_PLAYBACK_STATE_FAST_FORWARD,		/**< Fast forward */
	MC_PLAYBACK_STATE_REWIND,				/**< Rewind */
} mc_playback_states_e;

/**
 * @brief Enumeration for the shuffle mode.
 * @since_tizen 2.4
 */
typedef enum {
	MC_SHUFFLE_MODE_ON = 0,				/**< Shuffle mode on */
	MC_SHUFFLE_MODE_OFF,					/**< Shuffle mode off */
} mc_shuffle_mode_e;

/**
 * @brief Enumeration for the repeat mode.
 * @since_tizen 2.4
 */
typedef enum {
	MC_REPEAT_MODE_ON = 0,					/**< Repeat mode on */
	MC_REPEAT_MODE_OFF,					/**< Repeat mode off */
} mc_repeat_mode_e;

/**
 * @brief Enumeration for the subscription type.
 * @since_tizen 2.4
 */
typedef enum {
	MC_SUBSCRIPTION_TYPE_SERVER_STATE = 0,					/**< Server state */
	MC_SUBSCRIPTION_TYPE_PLAYBACK,						/**< Playback */
	MC_SUBSCRIPTION_TYPE_METADATA,						/**< Metadata */
	MC_SUBSCRIPTION_TYPE_SHUFFLE_MODE,					/**< Shuffle mode */
	MC_SUBSCRIPTION_TYPE_REPEAT_MODE,					/**< Repeat mode */
} mc_subscription_type_e;

/**
 * @}
 */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_MEDIA_CONTROLLER_TYPE_H__ */
