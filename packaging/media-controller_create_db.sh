#!/bin/sh

source /etc/tizen-platform.conf

mkdir -p $TZ_SYS_DB

#Create DB file
sqlite3 ${TZ_SYS_DB}/.media_controller.db 'PRAGMA journal_mode = PERSIST;'

#Change permission
chmod 664 ${TZ_SYS_DB}/.media_controller.db
chmod 664 ${TZ_SYS_DB}/.media_controller.db-journal

#Change owner
#chown -R :$TZ_SYS_USER_GROUP ${TZ_SYS_DATA}/data-media/*

#Change group (6017: db_filemanager 5000: app)
chgrp $TZ_SYS_USER_GROUP ${TZ_SYS_DB}
chgrp 6017 ${TZ_SYS_DB}/.media_controller.db
chgrp 6017 ${TZ_SYS_DB}/.media_controller.db-journal
