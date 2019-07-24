#!/bin/bash

read INSTR
# logger "${INSTR}"

GPIO_VALUE_PATH="/sys/class/gpio/gpio329/value"

VAL=`cat $GPIO_VALUE_PATH`

if [[ "$VAL" = "0" ]]; then
	
	echo 1 > $GPIO_VALUE_PATH
	echo '"enable"'

# else
	
# 	echo 0 > $GPIO_VALUE_PATH
# 	echo '"disable"'
	
fi
