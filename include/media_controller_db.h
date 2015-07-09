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

#ifndef __TIZEN_MEDIA_CONTROLLER_DB_H__
#define __TIZEN_MEDIA_CONTROLLER_DB_H__

#include <db-util.h>
#include <sqlite3.h>
#include <tzplatform_config.h>

#define MC_DB_NAME ".media_controller.db"

#define MC_DB_TABLE_SERVER_LIST		"server_list"
#define MC_DB_TABLE_LATEST_SERVER		"latest_server"

#define DB_DELETE_ALL_FROM_TABLE				"DELETE FROM '%q';"
#define DB_SELECT_META_FROM_TABLE				"SELECT * FROM '%q';"
#define DB_SELECT_SERVER_INFO_FROM_TABLE		"SELECT '%q' FROM '%q' WHERE '%q' = '%q';"
#define DB_SELECT_SERVER_STATE_FROM_TABLE		"SELECT server_state FROM '%q';"
#define DB_SELECT_PLAYBACK_STATE_FROM_DB		"SELECT playback_state FROM '%q';"
#define DB_SELECT_PLAYBACK_POSITION_FROM_DB	"SELECT playback_position FROM '%q';"
#define DB_SELECT_METADATA_FROM_DB			"SELECT * FROM '%q';"
#define DB_SELECT_SHUFFLE_MODE_FROM_DB		"SELECT shuffle_mode FROM '%q';"
#define DB_SELECT_REPEAT_MODE_FROM_DB		"SELECT repeat_mode FROM '%q';"
#define DB_SELECT_VALUE_OF_KEY					"SELECT %s FROM '%q';"

#define DB_UPDATE_PLAYBACK_INFO_INTO_SERVER_TABLE	"UPDATE '%q' SET playback_state=%d, playback_position=%llu;"
#define DB_UPDATE_METADATA_INTO_SERVER_TABLE			"UPDATE '%q' SET %s='%q';"
#define DB_UPDATE_SHUFFLE_MODE_INTO_SERVER_TABLE		"UPDATE '%q' SET shuffle_mode=%d;"
#define DB_UPDATE_REPEAT_MODE_INTO_SERVER_TABLE		"UPDATE '%q' SET repeat_mode=%d;"
#define DB_UPDATE_METADATA_INFO_INFO_SERVER_TABLE	"UPDATE '%q' SET title=%Q, artist=%Q, album=%Q, author=%Q, genre=%Q, duration=%Q, date=%Q, copyright=%Q, description=%Q, track_num=%Q, picture=%Q"

#define DB_INSERT_INTO_SERVER_TABLE			"INSERT INTO '%q' (server_name) VALUES ('%q');"
#define DB_DELETE_FROM_SERVER_TABLE			"DELETE FROM %q WHERE server_name = '%q';"
#define DB_UPDATE_VALUE_SERVER_TABLE		"UPDATE '%q' set '%q'='%q' where server_name='%q';"

#define DB_SELECT_LATEST_SERVER_NAME		"SELECT server_name FROM "MC_DB_TABLE_LATEST_SERVER";"
#define DB_SELECT_ALL_SERVER_LIST			"SELECT server_name FROM "MC_DB_TABLE_SERVER_LIST";"


#define DB_COLUMN_SERVER_NAME			"server_name"
#define DB_COLUMN_SERVER_STATE			"server_state"
#define DB_COLUMN_PLAYBACK_STATE			"playback_state"
#define DB_COLUMN_PLAYBACK_POSITION		"playback_position"

#define SQLITE3_SAFE_FREE(sql_string) 	{if(sql_string) { sqlite3_free(sql_string); sql_string = NULL;}}
#define SQLITE3_FINALIZE(x)	{if(x != NULL) {sqlite3_finalize(x);}}

int mc_db_connect(void **db_handle);
int mc_db_disconnect(void *db_handle);
int mc_db_create_tables(void *handle);
int mc_db_create_server_table(void *handle, const char *server_name);
int mc_db_delete_server_table(void *handle, const char *server_name);
int mc_db_check_server_table_exist(void *handle, const char *server_name, bool *exist);

int mc_db_update_playback_info(void *handle, const char *table_name, int playback_state, unsigned long long playback_position);
int mc_db_update_whole_metadata(void *handle, const char *server_name,
                                const char *title, const char *artist, const char *album, const char *author, const char *genre, const char *duration, const char *date,
                                const char *copyright, const char *description, const char *track_num, const char *picture);
int mc_db_update_shuffle_mode(void *handle, const char *table_name, int shuffle_mode);
int mc_db_update_repeat_mode(void *handle, const char *table_name, int repeat_mode);

int mc_db_get_latest_server_name(void *handle, char **latest_server_name);
int mc_db_get_server_state(void *handle, const char *server_name, mc_server_state_e *state);
int mc_db_get_playback_info(void *handle, const char *server_name, mc_playback_h *playback);
int mc_db_get_metadata_info(void *handle, const char *server_name, mc_metadata_h *metadata);
int mc_db_get_shuffle_mode(void *handle, const char *server_name, mc_shuffle_mode_e *mode);
int mc_db_get_repeat_mode(void *handle, const char *server_name, mc_repeat_mode_e *mode);

int mc_db_insert_server_address_into_table(void *db_handle, const char *table_name, char *address);
int mc_db_delete_server_address_from_table(void *db_handle, const char *table_name, char *address);

int mc_db_foreach_server_list(void *handle, mc_activated_server_cb callback, void *user_data);
int mc_db_update_server_state(void *handle, const char *server_name, mc_server_state_e server_state);
int mc_db_update_latest_server_table(void *handle, const char *server_name);


#endif /*__MEDIA_CONTROL_DB_H__*/
