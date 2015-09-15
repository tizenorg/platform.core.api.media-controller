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
#include <tzplatform_config.h>
#include <sys/stat.h>

#include "media_controller_private.h"
#include "media_controller_db_util.h"

#define FAT_FILEPATH_LEN_MAX		4096	/* inc null */
#define MC_FILE_PATH_LEN_MAX		FAT_FILEPATH_LEN_MAX		 /**< File path max length (include file name) on file system */

static int __mc_db_util_busy_handler(void *pData, int count)
{
	usleep(50000);

	mc_debug("mc_db_busy_handler called : %d", count);

	return 100 - count;
}

static char* __mc_get_db_name(uid_t uid)
{
	char result_psswd[MC_FILE_PATH_LEN_MAX];
	char *result_psswd_rtn = NULL;
	struct group *grpinfo = NULL;
	char * dir = NULL;

	memset(result_psswd, 0, sizeof(result_psswd));
	if(uid == getuid())
	{
		strncpy(result_psswd, MC_DB_NAME, sizeof(result_psswd));
		grpinfo = getgrnam("users");
		if(grpinfo == NULL) {
			mc_error("getgrnam(users) returns NULL !");
			return NULL;
		}
	}
	else
	{
		struct passwd *userinfo = getpwuid(uid);
		if(userinfo == NULL) {
			mc_error("getpwuid(%d) returns NULL !", uid);
			return NULL;
		}
		grpinfo = getgrnam("users");
		if(grpinfo == NULL) {
			mc_error("getgrnam(users) returns NULL !");
			return NULL;
		}
		// Compare git_t type and not group name
		if (grpinfo->gr_gid != userinfo->pw_gid) {
			mc_error("UID [%d] does not belong to 'users' group!", uid);
			return NULL;
		}
		snprintf(result_psswd, sizeof(result_psswd), "%s/.applications/dbspace/.media_controller.db", userinfo->pw_dir);
	}

	dir = strrchr(result_psswd, '/');
	if(!dir)
		return strdup(result_psswd);

	//Control if db exist create otherwise
	if(access(dir + 1, F_OK)) {
		int ret;
		mkdir(dir + 1, S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH);
		ret = chown(dir + 1, uid, grpinfo->gr_gid);
		if (ret == -1) {
			mc_debug("FAIL : chown %s %d.%d ", dir + 1, uid, grpinfo->gr_gid);
			mc_stderror("FAIL : chown");
		}
	}

	result_psswd_rtn = strdup(result_psswd);

	return result_psswd_rtn;
}

static int __mc_foreach_table_list(void *handle, void *user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	sqlite3 *db_handle = (sqlite3 *)handle;
	char *sql_str = NULL;
	sqlite3_stmt *stmt = NULL;
	GList **list = (GList **)(user_data);
	char *table_name = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	sql_str = sqlite3_mprintf("SELECT name FROM SQLITE_MASTER WHERE type='table'");
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
		table_name = (char *)sqlite3_column_text(stmt, 0);
		mc_debug("table_name: %s", table_name);
		if (MC_STRING_VALID(table_name)) {
			(*list) = g_list_append((*list), strdup(table_name));
		}
		ret = sqlite3_step(stmt);
	}

	SQLITE3_FINALIZE(stmt);
	SQLITE3_SAFE_FREE(sql_str);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

static int __mc_get_latest_server_name(void *handle, char **latest_server_name)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db_handle = (sqlite3 *)handle;
	char *server_name = NULL;

	mc_retvm_if(db_handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

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
		server_name = (char *)sqlite3_column_text(stmt, 0);
		if (MC_STRING_VALID(server_name)) {
			*latest_server_name = strdup(server_name);
		}
		ret = sqlite3_step(stmt);
	}

	SQLITE3_FINALIZE(stmt);
	SQLITE3_SAFE_FREE(sql_str);

	return MEDIA_CONTROLLER_ERROR_NONE;
}

static void __mc_delete_server_table_cb(gpointer data, gpointer user_data)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *table_name = (char*)data;
	void *handle = (void*)user_data;
	char *latest_server_name = NULL;

	mc_retm_if(handle == NULL, "Handle is NULL");
	mc_retm_if(table_name == NULL, "table name is NULL");

	ret = __mc_get_latest_server_name(handle, &latest_server_name);
	if (MEDIA_CONTROLLER_ERROR_NONE != ret) {
		mc_error("Error __mc_delete_server_table_cb %d", ret);
		return;
	}

	if (latest_server_name == NULL) {
		ret = mc_db_util_delete_server_table (handle, table_name);
		if (MEDIA_CONTROLLER_ERROR_NONE != ret) {
			mc_error("Error __mc_delete_server_table_cb %d", ret);
		}
	} else {
		mc_debug("Server name: %s, latest: %s", table_name, latest_server_name);

		if ((strcmp(latest_server_name, table_name) != 0) &&
			(strcmp(MC_DB_TABLE_LATEST_SERVER, table_name) != 0)) {
			ret = mc_db_util_delete_server_table (handle, table_name);
			if (MEDIA_CONTROLLER_ERROR_NONE != ret) {
				mc_error("Error __mc_delete_server_table_cb %d", ret);
			}
		}
		MC_SAFE_FREE(latest_server_name);
	}
}

static void __mc_destroy_table_list_cb(gpointer data)
{
	char *table_name = (char*)data;
	MC_SAFE_FREE(table_name);
}

static int __mc_create_latest_server_table(sqlite3 *handle)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	sql_str = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %q (server_name   TEXT PRIMARY KEY);", MC_DB_TABLE_LATEST_SERVER);

	ret = mc_db_util_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

static int __mc_create_server_list_table(sqlite3 *handle)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	sql_str = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				server_name   TEXT PRIMARY KEY);",
	                          MC_DB_TABLE_SERVER_LIST);

	ret = mc_db_util_update_db(handle, sql_str);

	SQLITE3_SAFE_FREE(sql_str);
	return ret;
}

int mc_db_util_connect(void **handle, uid_t uid, bool need_write)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	sqlite3 *db_handle = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	/*Connect DB*/
	if(need_write) {
		ret = db_util_open_with_options(__mc_get_db_name(uid), &db_handle, SQLITE_OPEN_READWRITE, NULL);
	} else {
		ret = db_util_open_with_options(__mc_get_db_name(uid), &db_handle, SQLITE_OPEN_READONLY, NULL);
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
	ret = sqlite3_busy_handler(db_handle, __mc_db_util_busy_handler, NULL);
	if (SQLITE_OK != ret) {
		mc_error("error when register busy handler %s\n", sqlite3_errmsg(db_handle));
		db_util_close(db_handle);
		*handle = NULL;

		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	*handle = db_handle;

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_util_update_db(void *handle, const char *sql_str)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *err_msg = NULL;
	sqlite3 *db_handle = (sqlite3 *)handle;
	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(sql_str == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "sql_str is NULL");

	mc_debug("Update query [%s]", sql_str);

	ret = sqlite3_exec(db_handle, sql_str, NULL, NULL, &err_msg);
	if (SQLITE_OK != ret) {
		mc_error("failed to update db[%s]", sqlite3_errmsg(db_handle));
		SQLITE3_SAFE_FREE(err_msg);
		return MEDIA_CONTROLLER_ERROR_INVALID_OPERATION;
	}

	return MEDIA_CONTROLLER_ERROR_NONE;
}

int mc_db_util_delete_server_table(void *handle, const char *server_name)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	char *sql_str = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");
	mc_retvm_if(server_name == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "server_name is NULL");

	sql_str = sqlite3_mprintf("DROP TABLE IF EXISTS '%q'", server_name);

	ret = mc_db_util_update_db(handle, sql_str);
	if (MEDIA_CONTROLLER_ERROR_NONE != ret) {
		mc_error("Error mc_db_util_update_db %d", ret);
	}

	SQLITE3_SAFE_FREE(sql_str);

	return ret;
}

int mc_db_util_delete_whole_server_tables(void *handle)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	GList *table_list = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = __mc_foreach_table_list(handle, &table_list);
	if (MEDIA_CONTROLLER_ERROR_NONE != ret) {
		mc_error("Error __mc_foreach_server_list %d", ret);
		return ret;
	}

	if (table_list == NULL) {
		mc_debug("No server list");
		return ret;
	}

	g_list_foreach(table_list, __mc_delete_server_table_cb, handle);

	g_list_free_full(table_list, __mc_destroy_table_list_cb);

	return ret;
}

int mc_db_util_disconnect(void *handle)
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

int mc_db_util_create_tables(void *handle)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	sqlite3 *db_handle = (sqlite3 *)handle;

	mc_retvm_if(db_handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	ret = __mc_create_latest_server_table(db_handle);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "create latest_server table failed!err= [%d]", ret);

	ret = __mc_create_server_list_table(db_handle);
	mc_retvm_if(ret != MEDIA_CONTROLLER_ERROR_NONE, ret, "create server_list table failed!err= [%d]", ret);

	return MEDIA_CONTROLLER_ERROR_NONE;
}
