#!/bin/bash

# ./server_on.sh
pipe=/tmp/rikbtnd.pipe
echo "switch power" > $pipe

# Вернуть состояние сервера
read line < $pipe
logger $line

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
