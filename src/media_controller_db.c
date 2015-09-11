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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <tzplatform_config.h>
#include <sys/stat.h>

#include "media_controller_private.h"
#include "media_controller_db.h"
#include "media_controller_socket.h"

typedef enum {
	MC_SERVER_FIELD_SERVER_NAME = 0,
	MC_SERVER_FIELD_SERVER_STATE,
	MC_SERVER_FIELD_PLAYBACK_STATE,
	MC_SERVER_FIELD_PLAYBACK_POSITION,
	MC_SERVER_FIELD_TITLE,
	MC_SERVER_FIELD_ARTIST,
	MC_SERVER_FIELD_ALBUM,
	MC_SERVER_FIELD_AUTHOR,
	MC_SERVER_FIELD_GENRE,
	MC_SERVER_FIELD_DURATION,
	MC_SERVER_FIELD_DATE,
	MC_SERVER_FIELD_COPYRIGHT,
	MC_SERVER_FIELD_DESCRIPTION,
	MC_SERVER_FIELD_TRACK_NUM,
	MC_SERVER_FIELD_PICTURE,
	MC_SERVER_FIELD_SHUFFLE_MODE,
	MC_SERVER_FIELD_REPEAT_MODE,
} server_table_field_e;

#define FAT_FILEPATH_LEN_MAX		4096	/* inc null */
#define MC_FILE_PATH_LEN_MAX		FAT_FILEPATH_LEN_MAX		 /**< File path max length (include file name) on file system */

static int __mc_db_busy_handler(void *pData, int count)
{
	usleep(50000);

	mc_debug("mc_db_busy_handler called : %d", count);

	return 100 - count;
}

static int __mc_db_update_db(void *handle, const char *sql_str)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;

	mc_retvm_if(!MC_STRING_VALID(sql_str), MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Invalid Query");

	ret = mc_ipc_send_message_to_server(MC_MSG_DB_UPDATE, sql_str);
	if (ret != MEDIA_CONTROLLER_ERROR_NONE) {
		mc_error("mc_ipc_send_message_to_server failed : %d", ret);
	}

	return ret;
}

static int __mc_db_create_latest_server_table(sqlite3 *handle)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	sql_str = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %q (server_name   TEXT PRIMARY KEY);", MC_DB_TABLE_LATEST_SERVER);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

static int __mc_db_create_server_list_table(sqlite3 *handle)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	sql_str = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				server_name   TEXT PRIMARY KEY);",
	                          MC_DB_TABLE_SERVER_LIST);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);
	return ret;
}

static int __mc_db_get_int_value_of_key(void *handle, const char *server_name, const char *key, int *value)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db_handle = (sqlite3 *)handle;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");
	mc_retvm_if(key == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "key is NULL");

	sql_str = sqlite3_mprintf(DB_SELECT_VALUE_OF_KEY, key, server_name);

	ret = sqlite3_prepare_v2(db_handle, sql_str, strlen(sql_str), &stmt, NULL);
	if (SQLITE_OK != ret) {
		mc_error("prepare error [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		mc_error("end of row [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_FINALIZE(stmt);
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}
	while (SQLITE_ROW == ret) {
		*value = sqlite3_column_int(stmt, 0);
		ret = sqlite3_step(stmt);
	}

	SQLITE3_FINALIZE(stmt);
	SQLITE3_SAFE_FREE(sql_str);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

static int __mc_db_get_ulong_value_of_key(void *handle, const char *server_name, const char *key, unsigned long long *value)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db_handle = (sqlite3 *)handle;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");
	mc_retvm_if(key == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "key is NULL");

	sql_str = sqlite3_mprintf(DB_SELECT_VALUE_OF_KEY, key, server_name);

	ret = sqlite3_prepare_v2(db_handle, sql_str, strlen(sql_str), &stmt, NULL);
	if (SQLITE_OK != ret) {
		mc_error("prepare error [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		mc_error("end of row [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_FINALIZE(stmt);
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}
	while (SQLITE_ROW == ret) {
		*value = (unsigned long long)sqlite3_column_int64(stmt, 0);
		ret = sqlite3_step(stmt);
	}

	SQLITE3_FINALIZE(stmt);
	SQLITE3_SAFE_FREE(sql_str);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_connect(void **handle, bool need_write)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	sqlite3 *db_handle = NULL;

	mc_error("mc_db_connect");

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	/*Connect DB*/
	if(need_write) {
		ret = db_util_open_with_options(tzplatform_mkpath(TZ_USER_DB, MC_DB_NAME), &db_handle, SQLITE_OPEN_READWRITE, NULL);
	} else {
		ret = db_util_open_with_options(tzplatform_mkpath(TZ_USER_DB, MC_DB_NAME), &db_handle, SQLITE_OPEN_READONLY, NULL);
	}
	if (SQLITE_OK != ret) {
		mc_error("error when db open");
		*handle = NULL;

		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	if (db_handle == NULL) {
		mc_error("*db_handle is NULL");
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	/*Register busy handler*/
	ret = sqlite3_busy_handler(db_handle, __mc_db_busy_handler, NULL);
	if (SQLITE_OK != ret) {
		mc_error("error when register busy handler %s\n", sqlite3_errmsg(db_handle));
		db_util_close(db_handle);
		*handle = NULL;

		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	*handle = db_handle;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_update_playback_info(void *handle, const char *server_name, int playback_state, unsigned long long playback_position)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf(DB_UPDATE_PLAYBACK_INFO_INTO_SERVER_TABLE, server_name, playback_state, playback_position);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_update_whole_metadata(void *handle, const char *server_name,
                                const char *title, const char *artist, const char *album, const char *author, const char *genre, const char *duration, const char *date,
                                const char *copyright, const char *description, const char *track_num, const char *picture)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf(DB_UPDATE_METADATA_INFO_INFO_SERVER_TABLE, server_name,
	                          title, artist, album, author, genre, duration, date, copyright, description, track_num, picture);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_update_shuffle_mode(void *handle, const char *server_name, int shuffle_mode)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf(DB_UPDATE_SHUFFLE_MODE_INTO_SERVER_TABLE, server_name, shuffle_mode);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_update_repeat_mode(void *handle, const char *server_name, int repeat_mode)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf(DB_UPDATE_REPEAT_MODE_INTO_SERVER_TABLE, server_name, repeat_mode);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_get_latest_server_name(void *handle, char **latest_server_name)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db_handle = (sqlite3 *)handle;
	char *server_name = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	*latest_server_name = NULL;

	sql_str = sqlite3_mprintf(DB_SELECT_LATEST_SERVER_NAME);

	ret = sqlite3_prepare_v2(db_handle, sql_str, strlen(sql_str), &stmt, NULL);
	if (SQLITE_OK != ret) {
		mc_error("prepare error [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		mc_error("end of row [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_FINALIZE(stmt);
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_NONE;	/*There is no activated server yet. */
	}

	while (SQLITE_ROW == ret) {
		MC_SAFE_FREE(server_name);
		server_name = strdup((char *)sqlite3_column_text(stmt, 0));
		ret = sqlite3_step(stmt);
	}

	if (server_name)
		*latest_server_name = server_name;

	SQLITE3_FINALIZE(stmt);
	SQLITE3_SAFE_FREE(sql_str);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_get_playback_info(void *handle, const char *server_name, mc_playback_h *playback)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	mc_playback_states_e playback_state = MEDIA_PLAYBACK_STATE_PLAYING;
	unsigned long long position = 0;
	media_controller_playback_s *_playback = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	ret = __mc_db_get_int_value_of_key(handle, server_name, "playback_state", (int *)&playback_state);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, MEDIA_CONTROLLER_ERROR_INVALID_OPERATION, "Fail to get playback_state");

	ret = __mc_db_get_ulong_value_of_key(handle, server_name, "playback_position", &position);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, MEDIA_CONTROLLER_ERROR_INVALID_OPERATION, "Fail to get position");

	_playback = (media_controller_playback_s *)calloc(1, sizeof(media_controller_playback_s));
	mc_retvm_if(_playback == NULL, MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

	_playback->state = playback_state;
	_playback->position = position;

	*playback = (mc_playback_h)_playback;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_get_metadata_info(void *handle, const char *server_name, mc_metadata_h *metadata)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db_handle = (sqlite3 *)handle;
	media_controller_metadata_s *_metadata = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf(DB_SELECT_METADATA_FROM_DB, server_name);

	ret = sqlite3_prepare_v2(db_handle, sql_str, strlen(sql_str), &stmt, NULL);
	if (SQLITE_OK != ret) {
		mc_error("prepare error [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}
	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		mc_error("end of row [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_FINALIZE(stmt);
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	} else {
		_metadata = (media_controller_metadata_s *)calloc(1, sizeof(media_controller_metadata_s));
		mc_retvm_if(_metadata == NULL, MEDIA_CONTROLLER_ERROR_OUT_OF_MEMORY, "OUT_OF_MEMORY");

		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_TITLE)))
			_metadata->title = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_TITLE));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_ARTIST)))
			_metadata->artist = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_ARTIST));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_ALBUM)))
			_metadata->album = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_ALBUM));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_AUTHOR)))
			_metadata->author = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_AUTHOR));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_GENRE)))
			_metadata->genre = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_GENRE));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_DURATION)))
			_metadata->duration = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_DURATION));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_DATE)))
			_metadata->date = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_DATE));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_COPYRIGHT)))
			_metadata->copyright = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_COPYRIGHT));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_DESCRIPTION)))
			_metadata->description = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_DESCRIPTION));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_TRACK_NUM)))
			_metadata->track_num = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_TRACK_NUM));
		if (MC_STRING_VALID((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_PICTURE)))
			_metadata->picture = strdup((const char *)sqlite3_column_text(stmt, MC_SERVER_FIELD_PICTURE));
	}

	*metadata = (mc_metadata_h)_metadata;

	SQLITE3_FINALIZE(stmt);
	SQLITE3_SAFE_FREE(sql_str);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_get_server_state(void *handle, const char *server_name, mc_server_state_e *state)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	ret = __mc_db_get_int_value_of_key(handle, server_name, "server_state", (int *)state);

	return ret;
}

int mc_db_get_shuffle_mode(void *handle, const char *server_name, mc_shuffle_mode_e *mode)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	ret = __mc_db_get_int_value_of_key(handle, server_name, "shuffle_mode", (int *)mode);

	return ret;
}

int mc_db_get_repeat_mode(void *handle, const char *server_name, mc_repeat_mode_e *mode)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	ret = __mc_db_get_int_value_of_key(handle, server_name, "repeat_mode", (int *)mode);

	return ret;
}

int mc_db_insert_server_address_into_table(void *handle, const char *table_name, char *address)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	sql_str = sqlite3_mprintf(DB_INSERT_INTO_SERVER_TABLE, table_name, address);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_delete_server_address_from_table(void *handle, const char *table_name, char *address)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	sql_str = sqlite3_mprintf(DB_DELETE_FROM_SERVER_TABLE, table_name, address);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_disconnect(void *handle)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	sqlite3 *db_handle = (sqlite3 *)handle;

	mc_retvm_if(db_handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = db_util_close(db_handle);
	if (SQLITE_OK != ret) {
		mc_error("error when db close");
		mc_error("Error : %s", sqlite3_errmsg(db_handle));
		db_handle = NULL;

		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_create_tables(void *handle)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	sqlite3 *db_handle = (sqlite3 *)handle;

	mc_retvm_if(db_handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = __mc_db_create_latest_server_table(db_handle);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "create latest_server table failed!err= [%d]", ret);

	ret = __mc_db_create_server_list_table(db_handle);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "create server_list table failed!err= [%d]", ret);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_create_server_table(void *handle, const char *server_name)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS '%q' (\
				server_name			TEXT PRIMARY KEY, \
				server_state			INTEGER DEFAULT 1, \
				playback_state		INTEGER DEFAULT 0, \
				playback_position	INTEGER DEFAULT 0, \
				title				TEXT, \
				artist				TEXT, \
				album				TEXT, \
				author				TEXT, \
				genre				TEXT, \
				duration			INTEGER DEFAULT 0, \
				date				TEXT, \
				copyright			TEXT, \
				description			TEXT, \
				track_num			TEXT, \
				picture				TEXT, \
				shuffle_mode		INTEGER DEFAULT 1, \
				repeat_mode			INTEGER DEFAULT 1 \
				);",
	                          server_name);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_delete_server_table(void *handle, const char *server_name)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf("DROP TABLE IF EXISTS '%q'", server_name);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_check_server_table_exist(void *handle, const char *server_name, bool *exist)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;
	sqlite3_stmt *stmt = NULL;
	int count = 0;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf("SELECT COUNT(*) FROM SQLITE_MASTER WHERE type='table' and name='%q'", server_name);

	ret = sqlite3_prepare_v2(handle, sql_str, strlen(sql_str), &stmt, NULL);
	if (SQLITE_OK != ret) {
		mc_error("prepare error [%s]\n", sqlite3_errmsg(handle));
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		mc_error("end of row [%s]\n", sqlite3_errmsg(handle));
		SQLITE3_FINALIZE(stmt);
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	count = sqlite3_column_int(stmt, 0);

	mc_debug("table count [%d]", count);

	if (count > 0)
		*exist = TRUE;
	else
		*exist = FALSE;

	SQLITE3_FINALIZE(stmt);
	SQLITE3_SAFE_FREE(sql_str);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_foreach_server_list(void *handle, mc_activated_server_cb callback, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	sqlite3 *db_handle = (sqlite3 *)handle;
	char *sql_str = NULL;
	sqlite3_stmt *stmt = NULL;

	mc_retvm_if(db_handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	sql_str = sqlite3_mprintf(DB_SELECT_ALL_SERVER_LIST);
	ret = sqlite3_prepare_v2(db_handle, sql_str, strlen(sql_str), &stmt, NULL);
	if (SQLITE_OK != ret) {
		mc_error("prepare error [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		mc_error("end of row [%s]\n", sqlite3_errmsg(db_handle));
		SQLITE3_FINALIZE(stmt);
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_NONE;
	}

	while (SQLITE_ROW == ret) {
		char *server_name = NULL;
		server_name = strdup((char *)sqlite3_column_text(stmt, 0));
		if (callback(server_name, user_data) == false) {
			MC_SAFE_FREE(server_name);
			break;
		}
		MC_SAFE_FREE(server_name);

		ret = sqlite3_step(stmt);
	}

	SQLITE3_FINALIZE(stmt);
	SQLITE3_SAFE_FREE(sql_str);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_update_server_state(void *handle, const char *server_name, mc_server_state_e server_state)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf("UPDATE '%q' SET server_state=%d;", server_name, server_state);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}


int mc_db_update_latest_server_table(void *handle, const char *server_name)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf("DELETE FROM '%q'; INSERT INTO '%q' (server_name) VALUES ('%q');", MC_DB_TABLE_LATEST_SERVER, MC_DB_TABLE_LATEST_SERVER, server_name);

	ret = __mc_db_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_check_server_registerd(void *handle, const char *server_name, bool *exist)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;
	sqlite3_stmt *stmt = NULL;
	int count = 0;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf(DB_SELECT_SERVER_COUNT, server_name);

	ret = sqlite3_prepare_v2(handle, sql_str, strlen(sql_str), &stmt, NULL);
	if (SQLITE_OK != ret) {
		mc_error("prepare error [%s]\n", sqlite3_errmsg(handle));
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	ret = sqlite3_step(stmt);
	if (SQLITE_ROW != ret) {
		mc_error("end of row [%s]\n", sqlite3_errmsg(handle));
		SQLITE3_FINALIZE(stmt);
		SQLITE3_SAFE_FREE(sql_str);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	count = sqlite3_column_int(stmt, 0);

	mc_debug("server count [%d]", count);

	if (count > 0)
		*exist = TRUE;
	else
		*exist = FALSE;

	SQLITE3_FINALIZE(stmt);
	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}
