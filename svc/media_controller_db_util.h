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

#ifndef __TIZEN_MEDIA_CONTROLLER_DB_UTIL_H__
#define __TIZEN_MEDIA_CONTROLLER_DB_UTIL_H__

#include <db-util.h>
#include <sqlite3.h>
#include <tzplatform_config.h>

#define MC_DB_NAME ".media_controller.db"

#define MC_DB_TABLE_SERVER_LIST		"server_list"
#define MC_DB_TABLE_LATEST_SERVER		"latest_server"

#define DB_SELECT_ALL_SERVER_LIST_EXCEPT_LATEST	"SELECT name FROM SQLITE_MASTER WHERE type='table' AND (name != '%q') AND (name NOT IN (SELECT server_name FROM "MC_DB_TABLE_LATEST_SERVER"))"

#define SQLITE3_SAFE_FREE(sql_string)	{if (sql_string) {sqlite3_free(sql_string); sql_string = NULL; } }
#define SQLITE3_FINALIZE(x)	{if (x != NULL) sqlite3_finalize(x); }

int mc_db_util_connect(void **handle, uid_t uid);
int mc_db_util_disconnect(void *handle);
int mc_db_util_create_tables(void *handle);
int mc_db_util_update_db(void *handle, const char *sql_str);
int mc_db_util_delete_whole_server_tables(void *handle);

#endif /*__TIZEN_MEDIA_CONTROLLER_DB_UTIL_H__*/
