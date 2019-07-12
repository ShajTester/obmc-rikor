#!/bin/bash

# ./server_on.sh
pipe=/tmp/rikbtnd.pipe

echo "switch power" > $pipe

# Вернуть состояние сервера

read line < $pipe

logger $line
# if [[ "$line" == 'on' ]]; then
#     echo '"on"'
# else
#     echo '"off"'
# fi

case $line in
	on)
		echo '"on"'
		;;
	off)
		echo '"off"'
		;;
	*)
		echo '"?"'
		;;
esac
