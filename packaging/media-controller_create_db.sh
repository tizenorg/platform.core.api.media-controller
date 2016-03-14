#!/bin/sh

source /etc/tizen-platform.conf

mkdir -p $TZ_USER_DB

#Create DB file
sqlite3 ${TZ_USER_DB}/.media_controller.db 'PRAGMA journal_mode = PERSIST;
		CREATE TABLE IF NOT EXISTS latest_server (server_name TEXT PRIMARY KEY);
'

#Change permission
chmod 644 ${TZ_USER_DB}/.media_controller.db
chmod 644 ${TZ_USER_DB}/.media_controller.db-journal

#Change group (6017: db_filemanager 5000: app)
chgrp $TZ_SYS_USER_GROUP ${TZ_USER_DB}
#chgrp 6017 ${TZ_USER_DB}/.media_controller.db
#chgrp 6017 ${TZ_USER_DB}/.media_controller.db-journal
