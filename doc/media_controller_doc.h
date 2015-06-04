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


#ifndef __TIZEN_MEDIA_CONTROLLER_DOC_H__
#define __TIZEN_MEDIA_CONTROLLER_DOC_H__

/**
 * @file media_controller_doc.h
 * @brief File contains the high level documentation for the Media Controller API.
 *
 */

/**
 * @defgroup CAPI_MEDIA_CONTROLLER_MODULE Media Controller
 * @ingroup CAPI_MEDIA_FRAMEWORK
 */

/**
 * @ingroup CAPI_MEDIA_FRAMEWORK
 * @addtogroup CAPI_MEDIA_CONTROLLER_MODULE
 * @brief The @ref CAPI_MEDIA_CONTROLLER_MODULE API provides functions for communication between the media controller server and the media controller client.
 * @section CAPI_MEDIA_CONTROLLER_MODULE_HEADER Required Header
 *    \#include <media_controller_server.h>
 *    \#include <media_controller_client.h>
 *
 * @section CAPI_MEDIA_CONTROLLER_OVERVIEW Overview
 * The @ref CAPI_MEDIA_CONTROLLER_MODULE API provides a set of functions to an effective communication between the server and the client for delivering the latest server information. It helps to transfer the information like playback info, shuffle mode, or the metadata of the latest server.  \n
 * To programming the interface, first, create a handler via #mc_client_create() or #mc_server_create(). And then, the client request the necessary information to the server by using #mc_client_set_server_update_callback(), or #mc_client_set_playback_update_callback(). The server provides the requested information to the client by the callback.
 *
 */

/**
* @ingroup CAPI_MEDIA_CONTROLLER_MODULE
* @defgroup CAPI_MEDIA_CONTROLLER_SERVER_MODULE Media Controller Server
* @brief The @ref CAPI_MEDIA_CONTROLLER_SERVER_MODULE API provides functions for sending the server information to the client.
* @section CAPI_MEDIA_CONTROLLER_SERVER_MODULE_HEADER Required Header
*	\#include <media_controller_server.h>
*
* @section CAPI_MEDIA_CONTROLLER_SERVER_MODULE_OVERVIEW Overview
* The @ref CAPI_MEDIA_CONTROLLER_SERVER_MODULE API allows you to send the playback info(#mc_server_set_playback_state(), #mc_server_set_playback_position), or the metadata(#mc_server_set_metadata()). and to receive the custom command(#mc_server_set_command_received_callback()), to update the status information on the latest server(#mc_server_update_playback_info, #mc_server_update_metadata), or to create/destroy the handle(#mc_server_create()/#mc_server_destroy()). \n
*
*/

/**
* @ingroup CAPI_MEDIA_CONTROLLER_MODULE
* @defgroup CAPI_MEDIA_CONTROLLER_CLIENT_MODULE Media Controller Client
* @brief The @ref CAPI_MEDIA_CONTROLLER_CLIENT_MODULE API provides functions for requesting the information to the server.
* @section CAPI_MEDIA_CONTROLLER_CLIENT_MODULE_HEADER Required Header
*	\#include <media_controller_client.h>
*
* @section CAPI_MEDIA_CONTROLLER_CLIENT_MODULE_OVERVIEW Overview
* The @ref CAPI_MEDIA_CONTROLLER_CLIENT_MODULE API allows you to check the activated server status(#mc_client_set_server_update_callback()), the playback info(#mc_client_set_playback_update_callback()), or the metadata(#mc_client_set_metadata_update_callback()). to send the custom command(#mc_client_send_custom_command()), to create/destroy the handle(#mc_client_create()/#mc_client_destroy()). \n
* If there is no the activated server, you can get the latest server information by using #mc_client_get_latest_server_info(), #mc_client_get_latest_playback_info(), or etc.
*
*/


#endif /* __TIZEN_MEDIA_CONTROLLER_DOC_H__ */


