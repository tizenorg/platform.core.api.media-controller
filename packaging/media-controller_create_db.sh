#!/bin/sh

source /etc/tizen-platform.conf

mkdir -p $TZ_USER_DB

#Create DB file
sqlite3 ${TZ_USER_DB}/.media_controller.db 'PRAGMA journal_mode = PERSIST;
		CREATE TABLE IF NOT EXISTS latest_server (server_name TEXT PRIMARY KEY);
'
