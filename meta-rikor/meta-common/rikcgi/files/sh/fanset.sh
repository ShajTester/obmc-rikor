#!/bin/bash

# Скрипт должен возвращать JSON c режимом управления вентиляторами,
# и заданием на PWM

PWMPATH="/sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm"

JSON="{\"fanauto\":\"off\""
JSON=$JSON",\"fan1\":\"45\""
JSON=$JSON",\"fan2\":\"45\""
JSON=$JSON",\"fan3\":\"45\""
JSON=$JSON",\"fan4\":\"45\""
JSON=$JSON",\"fan5\":\"45\""
JSON=$JSON",\"fan6\":\"45\""
JSON=$JSON",\"fan7\":\"45\""
JSON=$JSON",\"fan8\":\"45\""
JSON=$JSON"}"

echo $JSON


