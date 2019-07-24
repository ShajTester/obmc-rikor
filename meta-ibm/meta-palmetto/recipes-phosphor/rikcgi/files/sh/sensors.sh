#!/bin/bash

function get_sensor1
{
   S1_DIR=/sys/class/i2c-dev/i2c-6/device/6-0049/hwmon/hwmon1
   set -e

   echo "$(cat $S1_DIR/temp1_input)"
}

function get_sensor2
{
   S2_DIR=/sys/class/i2c-dev/i2c-6/device/6-004b/hwmon/hwmon4
   set -e

   echo "$(cat $S2_DIR/temp1_input)"
}

function get_sensor3
{
   S3_DIR=/sys/class/i2c-dev/i2c-6/device/6-004c/hwmon/hwmon3
   set -e

   echo "$(cat $S3_DIR/temp1_input)"
}

function get_sensor4
{
   S4_DIR=/sys/class/i2c-dev/i2c-6/device/6-004e/hwmon/hwmon2
   set -e

   echo "$(cat $S4_DIR/temp1_input)"
}

function get_sensor5
{
   S5_DIR=/sys/class/i2c-dev/i2c-6/device/6-004f/hwmon/hwmon5
   set -e

   echo "$(cat $S5_DIR/temp1_input)"
}

SVAL1=`get_sensor1` || exit 1
SVAL2=`get_sensor2` || exit 1
SVAL3=`get_sensor3` || exit 1
SVAL4=`get_sensor4` || exit 1
SVAL5=`get_sensor5` || exit 1

VAL1=$(( $SVAL1 / 1000 )).$(( ($SVAL1 % 1000) / 100 ))°C
VAL2=$(( $SVAL2 / 1000 )).$(( ($SVAL2 % 1000) / 100 ))°C
VAL3=$(( $SVAL3 / 1000 )).$(( ($SVAL3 % 1000) / 100 ))°C
VAL4=$(( $SVAL4 / 1000 )).$(( ($SVAL4 % 1000) / 100 ))°C
VAL5=$(( $SVAL5 / 1000 )).$(( ($SVAL5 % 1000) / 100 ))°C

STR="[[\"Sensor1\",\"$VAL1\"],[\"Sensor2\",\"$VAL2\"],[\"Sensor3\",\"$VAL3\"],[\"Sensor4\",\"$VAL4\"],[\"Sensor5\",\"$VAL5\"]]"

echo $STR
