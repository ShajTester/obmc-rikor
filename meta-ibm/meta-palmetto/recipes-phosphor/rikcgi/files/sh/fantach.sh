#!/bin/bash

function get_speed
{
  num=$1
  PWM_DIR=/sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0
  set -e

  echo "$(cat $PWM_DIR/pwm${num}_input)"
}

TACH1=`get_speed 0` || exit 2
TACH2=`get_speed 1` || exit 2
TACH3=`get_speed 2` || exit 2
TACH4=`get_speed 3` || exit 2
TACH5=`get_speed 4` || exit 2
TACH6=`get_speed 5` || exit 2
TACH7=`get_speed 6` || exit 2
TACH8=`get_speed 7` || exit 2

STR="[[\"Fan1\",\"$TACH1\"],[\"Fan2\",\"$TACH2\"],[\"Fan3\",\"$TACH3\"],[\"Fan4\",\"$TACH4\"],[\"Fan5\",\"$TACH5\"],[\"Fan6\",\"$TACH6\"],[\"Fan7\",\"$TACH7\"],[\"Fan8\",\"$TACH8\"]]"

echo $STR



