#!/bin/bash

read INSTR
# logger "${INSTR}"

VAL=`cat /sys/class/gpio/gpio322/value`

if [[ "$VAL" = "0" ]]; then
	
	echo 1 > /sys/class/gpio/gpio322/value
	echo '"off"'

else
	
	echo 0 > /sys/class/gpio/gpio322/value
	echo '"on"'
	
fi
