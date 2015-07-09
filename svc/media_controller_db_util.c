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

int mc_db_util_connect(void **handle, uid_t uid)
{
	int ret = MEDIA_CONTROLLER_ERROR_NONE;
	sqlite3 *db_handle = NULL;

	mc_retvm_if(handle == NULL, MEDIA_CONTROLLER_ERROR_INVALID_PARAMETER, "Handle is NULL");

	/*Connect DB*/
	ret = db_util_open(__mc_get_db_name(uid), &db_handle, DB_UTIL_REGISTER_HOOK_METHOD);
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
