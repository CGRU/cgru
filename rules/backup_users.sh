#!/usr/bin/env bash
#
# /etc/crontab
# 5 5 * * * /data/cgru/rules/backup_users.sh
# Every day at 05:05 AM
#

cd `dirname "$0"`
cd ..
cd "users"


for ufolder in *; do
	if [ ! -d "${ufolder}" ]; then
		continue
	fi
	cd "${ufolder}"
	for ufile in *.json; do
		if [ ! -f "${ufile}" ]; then
			continue
		fi
		cp -vf "${ufile}" "${ufile}.bak"
	done
	cd ..
done

