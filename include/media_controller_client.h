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

#ifndef __TIZEN_MEDIA_CONTROLLER_CLIENT_H__
#define __TIZEN_MEDIA_CONTROLLER_CLIENT_H__

#include <media_controller_type.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file media_controller_client.h
 * @brief This file contains the media controller client API and functions related with handling media control. \n
 *        Functions include operations to get the latest status of the media controller servers.
 */

/**
 * @addtogroup CAPI_MEDIA_CONTROLLER_CLIENT_MODULE
 * @{
 */

/**
 * @brief Called when updating status of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks The callback is called in a separate process.
 *
 * @param[in] server_name,    The app_id of the updated media controller server
 * @param[in] state,    The state of the updated media controller server
 * @param[in] user_data        The user data passed from the mc_client_set_server_update_cb() fuction
 *
 * @pre mc_client_set_server_update_cb()
 *
 * @see mc_client_set_server_update_cb()
 */
typedef void (*mc_server_state_updated_cb)(const char *server_name, mc_server_state_e state, void *user_data);

/**
 * @brief Called when updating the playback information of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks The callback is called in a separate process.
 *
 * @param[in] server_name,    The app_id of the updated media controller server
 * @param[in] playback,        The playback information of the updated media controller server
 * @param[in] user_data        The user data passed from the mc_client_set_playback_update_cb() fuction
 *
 * @pre mc_client_set_playback_update_cb()
 *
 * @see mc_client_set_playback_update_cb()
 */
typedef void (*mc_playback_updated_cb)(const char *server_name, mc_playback_h playback, void *user_data);

/**
 * @brief Called when updating the metadata of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks The callback is called in a separate process.
 *
 * @param[in] server_name,    The app_id of the updated media controller server
 * @param[in] metadata,        the metadata of the updated media controller server
 * @param[in] user_data        The user data passed from the mc_client_set_metadata_update_cb() fuction
 *
 * @pre mc_client_set_metadata_update_cb()
 *
 * @see mc_client_set_metadata_update_cb()
 */
typedef void (*mc_metadata_updated_cb)(const char *server_name, mc_metadata_h metadata, void *user_data);

/**
 * @brief Called when updating the shuffle mode of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks The callback is called in a separate process.
 *
 * @param[in] server_name,    The app_id of the updated media controller server
 * @param[in] mode,            The shuffle mode of the updated media controller server
 * @param[in] user_data        The user data passed from the mc_client_set_shuffle_mode_update_cb() fuction
 *
 * @pre mc_client_set_shuffle_mode_update_cb()
 *
 * @see mc_client_set_shuffle_mode_update_cb()
 */
typedef void (*mc_shuffle_mode_changed_cb)(const char *server_name, mc_shuffle_mode_e mode, void *user_data);

/**
 * @brief Called when updating the repeat mode of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks The callback is called in a separate process.
 *
 * @param[in] server_name,    The app_id of the updated media controller server
 * @param[in] mode,            The repeat mode of the updated media controller server
 * @param[in] user_data        The user data passed from the mc_client_set_repeat_mode_update_cb() fuction
 *
 * @pre mc_client_set_repeat_mode_update_cb()
 *
 * @see mc_client_set_repeat_mode_update_cb()
 */
typedef void (*mc_repeat_mode_changed_cb)(const char *server_name, mc_repeat_mode_e mode, void *user_data);

/**
 * @brief Called when requesting the list of activated servers.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks The callback is called in a separate process.
 *
 * @param[in] server_name,    The app_id of the activated media controller server
 * @param[in] user_data        The user data passed from the mc_client_foreach_server() fuction
 *
 * @return @c true to continue with the next iteration of the loop,
 *         otherwise @c false to break out of the loop
 *
 * @pre mc_client_foreach_server()
 *
 * @see mc_client_foreach_server()
 */
typedef bool (*mc_activated_server_cb)(const char *server_name, void *user_data);

/**
 * @brief Called when receiving the command processing result from the server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks The callback is called in a separate process.
 *
 * @param[in] server_name    The app_id of the updated media controller server
 * @param[in] result_code    The result code of custom command
 * @param[in] data The extra data
 * @param[in] user_data        The user data passed from the mc_client_send_custom_command() fuction
 *
 * @pre mc_client_send_custom_command()
 *
 * @see mc_client_send_custom_command()
 */
typedef void (*mc_command_reply_received_cb)(const char *server_name, int result_code, bundle *data, void *user_data);

/**
 * @brief Called when requesting the list of subscribed servers.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks The callback is called in a separate process.
 *
 * @param[in] server_name,    The app_id of the subscribed media controller server
 * @param[in] user_data        The user data passed from the mc_client_foreach_server_subscribed() fuction
 *
 * @return @c true to continue with the next iteration of the loop,
 *         otherwise @c false to break out of the loop
 *
 * @pre mc_client_foreach_server_subscribed()
 *
 * @see mc_client_foreach_server_subscribed()
 */
typedef bool (*mc_subscribed_server_cb)(const char *server_name, void *user_data);

/**
 * @brief Creates a media controller client.
 * @details The media controller client binds the latest media controller server when handlers are created.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @remarks You must release @a client using mc_client_destroy().
 *
 * @param[out] client The handle to the media controller client
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @see mc_client_destroy()
 */
int mc_client_create(mc_client_h *client);

/**
 * @brief Sets the callback for monitoring status of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details If media controller client call this function, basically the media controller client recieve the callback from all media controller servers.
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client         The handle to the media controller client
 * @param [in] callback      The callback to be invoked when the media controller server status is changed
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_set_server_update_cb(mc_client_h client, mc_server_state_updated_cb callback, void *user_data);

/**
 * @brief Unsets the callback for monitoring status of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client         The handle to the media controller client
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_unset_server_update_cb(mc_client_h client);

/**
 * @brief Sets the callback for monitoring playback status of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details If media controller client call this function, basically the media controller client recieve the callback from all media controller servers. \n
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client         The handle to the media controller client
 * @param [in] callback      The callback to be invoked when the playback status is changed
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_set_playback_update_cb(mc_client_h client, mc_playback_updated_cb callback, void *user_data);

/**
 * @brief Unsets the callback for monitoring playback status of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client         The handle to the media controller client
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_unset_playback_update_cb(mc_client_h client);

/**
 * @brief Sets the callback for monitoring metadata status of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details If media controller client call this function, basically the media controller client recieve the callback from all media controller servers.
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client         The handle to the media controller client
 * @param [in] callback      The callback to be invoked when the metadata status is changed
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_set_metadata_update_cb(mc_client_h client, mc_metadata_updated_cb callback, void *user_data);

/**
 * @brief Unsets the callback for monitoring metadata status of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client         The handle to the media controller client
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_unset_metadata_update_cb(mc_client_h client);

/**
 * @brief Sets the callback for monitoring shuffle mode of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details If media controller client call this function, basically the media controller client recieve the callback from all media controller servers.
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] callback      The callback to be invoked when the shuffle mode is changed
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_set_shuffle_mode_update_cb(mc_client_h client, mc_shuffle_mode_changed_cb callback, void *user_data);

/**
 * @brief Unsets the callback for monitoring shuffle mode of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_unset_shuffle_mode_update_cb(mc_client_h client);

/**
 * @brief Sets the callback for monitoring repeat mode of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details If media controller client call this function, basically the media controller client recieve the callback from all media controller servers.
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] callback      The callback to be invoked when the repeat mode is changed
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_set_repeat_mode_update_cb(mc_client_h client, mc_repeat_mode_changed_cb callback, void *user_data);

/**
 * @brief Unsets the callback for monitoring repeat mode of the media controller server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_destroy()
 */
int mc_client_unset_repeat_mode_update_cb(mc_client_h client);

/**
 * @brief Subscribes media controller server for monitoring status.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details If media controller client subscribe media controller server, \n
 *              the media controller client recieve callback from subscribed media controller server. \n
 *              If media controller client subscribe media controller server one or more, \n
 *              the media controller client can recieve callback from only subscribed media controller server. \n
 *              If you want to subscribe for the all media controller server again, \n
 *              unset mode update callback and set the callback for the monitoring status again. \n
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] subscription_type    The subscription type
 * @param [in] server_name    The app_id of the media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Create a media controller client handle by calling mc_client_create()
 * @pre Set the callback for monitoring status of the media controller server
 * @post Unsubscribe the media controller server for monitoring status by calling mc_client_unsubscribe()
 * @see mc_client_create()
 * @see mc_client_unsubscribe()
 */
int mc_client_subscribe(mc_client_h client, mc_subscription_type_e subscription_type, const char *server_name);

/**
 * @brief Unsubscribes media controller server for monitoring status.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details If media controller client unsubscribe media controller server, \n
 *              the media controller client don't recieve callback from unsubscribed media controller server. \n
 *              If media controller client unsubscribe all subscibed media controller server, \n
 *              the media controller client don't recieve callback from all media controller server. \n
 *              After unset and set update callback function is called again, the media controller client can recieve callback from all media controller servers. \n
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] subscription_type    The subscription type
 * @param [in] server_name    The app_id of the media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Create a media controller client handle by calling mc_client_create()
 * @pre Subscribe the media controller server for monitoring status by calling mc_client_subscribe()
 * @see mc_client_create()
 * @see mc_client_subscribe()
 */
int mc_client_unsubscribe(mc_client_h client, mc_subscription_type_e subscription_type, const char *server_name);

/**
 * @brief Retrieves all subscribed Server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] subscription_type    The subscription type
 * @param [in] callback      The callback to be invoked when the list of the subscribed media controller server.
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Create a media controller client handle by calling mc_client_create()
 * @pre Subscribe the media controller server for monitoring status by calling mc_client_subscribe()
 * @see mc_client_create()
 * @see mc_client_subscribe()
 */
int mc_client_foreach_server_subscribed(mc_client_h client, mc_subscription_type_e subscription_type, mc_subscribed_server_cb callback, void *user_data);

/**
 * @brief Gets the playback state.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks You must release @a playback using @c mc_client_destroy_playback().
 *
 * @param [in] playback    The handle to playback
 * @param [out] state      The state of the playback
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Sets mc_client_set_playback_update_cb() function to get the state
 * @see mc_client_set_playback_update_cb()
 */
int mc_client_get_playback_state(mc_playback_h playback, mc_playback_states_e *state);

/**
 * @brief Gets the playback position.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks You must release @a playback using @c mc_client_destroy_playback().
 *
 * @param [in] playback    The handle to playback
 * @param [out] position      The position of the playback in milliseconds
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Sets mc_client_set_playback_update_cb() function to get the position
 * @see mc_client_set_playback_update_cb()
 */
int mc_client_get_playback_position(mc_playback_h playback, unsigned long long *position);

/**
 * @brief Destroys playback
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @param [in] playback    The handle to playback
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Sets mc_client_set_playback_update_cb() function to create playback
 * @see mc_client_set_playback_update_cb()
 */
int mc_client_destroy_playback(mc_playback_h playback);

/**
 * @brief Gets the metadata.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @remarks You must release @a metadata using @c mc_client_destroy_metadata(). \n
 *               And also You must release @a value using free().
 *                   If the attribute value of the metadata is empty, return value is NULL.
 *
 * @param [in] metadata    The handle to metadata
 * @param [in] attribute    The key attribute name to get
 * @param [out] value      The value of the attribute
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Sets mc_client_set_metadata_update_cb() function to get the metadata
 * @see mc_client_set_metadata_update_cb()
 */
int mc_client_get_metadata(mc_metadata_h metadata, mc_meta_e attribute, char **value);

/**
 * @brief Destroys metadata
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @param [in] playback    The handle to metadata
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Sets mc_client_set_metadata_update_cb() function to create metadata
 * @see mc_client_set_metadata_update_cb()
 */
int mc_client_destroy_metadata(mc_metadata_h metadata);

/**
 * @brief Gets the latest media controller server info.
 * @details The media controller client will get the most recently updated information by the server.
 *
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @remarks If there is no activated media controller server, return value of the server name is NULL.
 *
 * @param [in] client    The handle to the media controller client
 * @param [out] server_name    The app_id of the latest media controller server
 * @param [out] server_state      The state of the latest media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 */
int mc_client_get_latest_server_info(mc_client_h client, char **server_name, mc_server_state_e *server_state);

/**
 * @brief Gets the latest playback info.
 * @details The media controller client will get the most recently updated information from @a server_name.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @remarks You must release @a playback using @c mc_client_destroy_playback(). \n
 *                   If there is no playback info, return value of the playback is NULL.
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] server_name    The app_id of the server to requesting
 * @param [out] playback    The handle to playback
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 */
int mc_client_get_server_playback_info(mc_client_h client, const char *server_name, mc_playback_h *playback);

/**
 * @brief Gets the latest metadata.
 * @details The media controller client will get the most recently updated information from @a server_name.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @remarks You must release @a metadata using @c mc_client_destroy_metadata(). \n
 *                   If there is no metadata, return value of the metadata is NULL.
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] server_name    The app_id of the server to requesting
 * @param [out] metadata    The handle to metadata
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 */
int mc_client_get_server_metadata(mc_client_h client, const char *server_name, mc_metadata_h *metadata);

/**
 * @brief Gets the latest shuffle mode.
 * @details The media controller client will get the most recently updated information from @a server_name.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @remarks If there is no shuffle mode info, return value is SHUFFLE_MODE_OFF.
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] server_name    The app_id of the server to requesting
 * @param [out] mode    The info of the latest shuffle mode
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 */
int mc_client_get_server_shuffle_mode(mc_client_h client, const char *server_name, mc_shuffle_mode_e *mode);

/**
 * @brief Gets the latest repeat mode.
 * @details The media controller client will get the most recently updated information from @a server_name.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @remarks If there is no repeat mode info, return value is REPEAT_MODE_OFF.
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] server_name    The app_id of the server to requesting
 * @param [out] mode    The info of the latest shuffle mode
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 */
int mc_client_get_server_repeat_mode(mc_client_h client, const char *server_name, mc_repeat_mode_e *mode);

/**
 * @brief Retrieves all activated Server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] callback      The callback to be invoked when the list of the registered media controller server created completely.
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 */
int mc_client_foreach_server(mc_client_h client, mc_activated_server_cb callback, void *user_data);

/**
 * @brief Sends the playback state command to server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] server_name    The app_id of the media controller server
 * @param [in] state   The playback state command to send media controller server
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 * @see mc_client_get_server_playback_info()
 */
int mc_client_send_playback_state_command(mc_client_h client, const char *server_name, mc_playback_states_e state);

/**
 * @brief Sends the custom command to server.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @details If there is the result for comand from server, the media controller client will get the result of the custom command by mc_command_reply_received_cb() callback.
 *
 * @privlevel public
 * @privilege %http://tizen.org/privilege/mediacontroller.client
 *
 * @param [in] client    The handle to the media controller client
 * @param [in] server_name    The app_id of the media controller server
 * @param [in] command      The command to be sent
 * @param [in] data      The extra data
 * @param [in] callback      The callback to be invoked when the custom command execute completely.
 * @param [in] user_data   The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #MEDIA_CONTROLLER_ERROR_NONE Successful
 * @retval #MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #MEDIA_CONTROLLER_ERROR_PERMISSION_DENIED Permission denied
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 */
int mc_client_send_custom_command(mc_client_h client, const char *server_name, const char *command, bundle *data, mc_command_reply_received_cb callback, void *user_data);

/**
 * @brief Destroys client.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param [in] client The handle to the media controller client
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #METADATA_EXTRACTOR_ERROR_NONE Successful
 * @retval #METADATA_EXTRACTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre Create a media controller client handle by calling mc_client_create().
 * @see mc_client_create()
 */
int mc_client_destroy(mc_client_h client);


/**
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIZEN_MEDIA_CONTROLLER_CLIENT_H__ */
