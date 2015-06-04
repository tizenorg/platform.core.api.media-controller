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

#ifndef __TIZEN_MEDIA_CONTROLLER_SERVER_H__
#define __TIZEN_MEDIA_CONTROLLER_SERVER_H__

#include <media_controller_type.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file media_controller_server.h
 * @brief This file contains the media controller server API and functions related with handling media controller server. \n
 *        Functions include operations to send the latest status of the server to client.
 */

/**
 * @addtogroup CAPI_MEDIA_CONTROLLER_SERVER_MODULE
 * @{
 */

/**
 * @brief Called when the Server received playback state command from the client.
 * @since_tizen 2.4
 *
 * @details This callback is called when server received playback state recommand from client.
 *
 * @param[in] client_name    The app_id of the media controller client
 * @param[in] state    The received playback state
 * @param[in] user_data        The user data passed from the mc_server_set_playback_state_command_received_cb() fuction
 *
 * @pre mc_server_set_playback_state_received_cb()
 *
 * @see mc_server_set_playback_state_received_cb()
 */
typedef void (*mc_server_playback_state_command_received_cb)(const char* client_name, mc_playback_states_e state, void *user_data);

/**
 * @brief Called when the Server received custom command from the client.
 * @since_tizen 2.4
 *
 * @details This callback is called when server received custom recommand from client.
 *                If there is reply for command, call mc_server_send_command_reply() function.
 *
 * @param[in] client_name    The app_id of the media controller client
 * @param[in] command    The received command
 * @param[in] data    The extra data
 * @param[in] user_data        The user data passed from the mc_server_set_custom_command_received_cb() fuction
 *
 * @pre mc_server_set_command_received_cb()
 *
 * @see mc_server_set_command_received_cb()
 */
typedef void (*mc_server_custom_command_received_cb)(const char* client_name, const char *command, bundle *data, void *user_data);

/**
 * @brief Creates a media controller server.
 * @since_tizen 2.4
 * @remarks You must release @a server using mc_server_destroy().
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param[out] server The handle to media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @see mc_server_destroy()
 */
int mc_server_create(mc_server_h *server);

/**
 * @brief Sets the playback state to update the latest state info
 * @since_tizen 2.4
 * @param [in] server The handle to media controller server
 * @param [in] state The state to set
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY File does not exist
 * @pre Create a media controller server handle by calling mc_server_create().
 * @post Apply the updated playback information by calling mc_server_update_playback_info().
 * @see mc_server_create()
 * @see mc_server_destroy()
 * @see mc_server_update_playback_info()
 */
int mc_server_set_playback_state(mc_server_h server, mc_playback_states_e state);

/**
 * @brief Sets the playback position to update the latest playback info
 * @since_tizen 2.4
 * @param [in] server The handle to media controller server
 * @param [in] position The position to set in milliseconds
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY File does not exist
 * @pre Create a media controller server handle by calling mc_server_create().
 * @post Apply the updated playback information by calling mc_server_update_playback_info().
 * @see mc_server_create()
 * @see mc_server_destroy()
 * @see mc_server_update_playback_info()
 */
int mc_server_set_playback_position(mc_server_h server, unsigned long long position);

/**
 * @brief Update the modified playback info
 * @details If this API is called, the updated playback information will be sent to the controller.
 * @since_tizen 2.4
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server The handle to media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY File does not exist
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @see mc_server_create()
 * @see mc_server_destroy()
 */
int mc_server_update_playback_info(mc_server_h server);

/**
 * @brief Sets the metadata to update the latest metadata info
 * @since_tizen 2.4
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server The handle to media controller server
 * @param [in] attribute    The key attribute name to set
 * @param [in] value      The value of the attribute
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY File does not exist
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @post Apply the updated metadata information by calling mc_server_update_metadata().
 * @see mc_server_create()
 * @see mc_server_destroy()
 * @see mc_server_update_metadata()
 */
int mc_server_set_metadata(mc_server_h server, mc_meta_e attribute, const char *value);

/**
 * @brief Update the modified metadata info.
 * @details If this API is called, the updated metadata will be sent to all controllers.
 * @since_tizen 2.4
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server The handle to media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY File does not exist
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @see mc_server_create()
 * @see mc_server_destroy()
 */
int mc_server_update_metadata(mc_server_h server);

/**
 * @brief Update the modified shuffle mode
 * @details If this API is called, the updated mode information will be sent to all controllers.
 * @since_tizen 2.4
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server The handle to media controller server
 * @param [in] mode The shuffle mode to update the latest status
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY File does not exist
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @see mc_server_create()
 * @see mc_server_destroy()
 */
int mc_server_update_shuffle_mode(mc_server_h server, mc_shuffle_mode_e mode);

/**
 * @brief Updates the modified repeat mode
 * @details If this API is called, the updated mode information will be sent to all controllers.
 * @since_tizen 2.4
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server The handle to media controller server
 * @param [in] mode The repeat mode to update the latest status
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY File does not exist
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @see mc_server_create()
 * @see mc_server_destroy()
 */
int mc_server_update_repeat_mode(mc_server_h server, mc_repeat_mode_e mode);

/**
 * @brief Sets the callback for receiving playback state command from client.
 * @since_tizen 2.4
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server    The handle to media controller server
 * @param [in] callback      The callback to be invoked when media controller server receives playback command from client.
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @see mc_server_create()
 * @see mc_server_destroy()
 */
int mc_server_set_playback_state_command_received_cb(mc_server_h server, mc_server_playback_state_command_received_cb callback, void *user_data);

/**
 * @brief Unsets the callback for receiving playback state command from client.
 * @since_tizen 2.4
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server    The handle to media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @see mc_server_create()
 * @see mc_server_destroy()
 */
int mc_server_unset_playback_state_command_received_cb(mc_server_h server);

/**
 * @brief Sets the callback for receiving custom command from client.
 * @since_tizen 2.4
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server    The handle to media controller server
 * @param [in] callback      The callback to be invoked when media controller server receives custom command from client.
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @see mc_server_create()
 * @see mc_server_destroy()
 */
int mc_server_set_custom_command_received_cb(mc_server_h server, mc_server_custom_command_received_cb callback, void *user_data);

/**
 * @brief Unsets the callback for receiving custom command from client.
 * @since_tizen 2.4
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server    The handle to media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @see mc_server_create()
 * @see mc_server_destroy()
 */
int mc_server_unset_custom_command_received_cb(mc_server_h server);

/**
 * @brief Sends a reply for the requested command to the client.
 * @since_tizen 2.4
 * @remarks When server recieve command, this API can be called by mc_server_custom_command_received_cb().
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.server
 *
 * @param [in] server    The handle to media controller server
 * @param [in] client_name    The app_id of the media controller client
 * @param [in] result_code    The result code of custom command
 * @param [in] data                The extra data
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller server handle by calling mc_server_create().
 * @pre mc_server_command_received_cb()
 * @see mc_server_create()
 * @see mc_server_destroy()
 */
int mc_server_send_command_reply(mc_server_h server, const char *client_name, int result_code, bundle *data);

/**
 * @brief Destroys media controller server.
 * @since_tizen 2.4
 * @param [in] server The handle to media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #METADATA_EXTRACTOR_ERROR_NONE Successful
 * @retval #METADATA_EXTRACTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Create a media controller server handle by calling mc_server_create().
 * @see mc_server_create()
 */
int mc_server_destroy(mc_server_h server);	//deregistered internally


/**
 * @}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_MEDIA_CONTROLLER_SERVER_H__ */
