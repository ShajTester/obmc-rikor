

#define _GNU_SOURCE

#include <syslog.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/ioctl.h>

#include <dirent.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include <libfastjson/json.h>

// #define DEBUG_MODE


int main(void)
{
	// int tmp = 0;
	int i;

    openlog("rikcgi-pch", LOG_CONS, LOG_USER);

	int file;
	int adapter_nr = 3; /* probably dynamically determined */
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
	file = open(filename, O_RDWR);
	if (file < 0) {
		/* ERROR HANDLING; you can check errno to see what went wrong */
		syslog(LOG_ERR, "Device open error %d\n", errno);
		exit(1);
	}


	int addr = 0x44; /* The I2C address */

	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		/* ERROR HANDLING; you can check errno to see what went wrong */
		syslog(LOG_ERR, "Add slave error %d\n", errno);
		exit(1);
	}


	__u8 res[16];

	for(i=0; i<16; i++)
	{
		res[i] = i2c_smbus_read_byte_data(file, i);
		if (res < 0) {
			/* ERROR HANDLING: i2c transaction failed */
			syslog(LOG_ERR, "Read register %d error %d\n", i, res[i]);
		}
	}


	fjson_object *root_object, *reg4_object, *reg5_object;
	// fjson_object *my_string, *my_int, *my_array;

	char strval[80];

	root_object = fjson_object_new_object();

	// Register 1
	// System Power State
	switch(res[1]) {
		case 0:
		case 1:
		case 3:
		case 4:
		case 5:
			sprintf(strval, "S%d", res[1]);
			break;

		default:
			sprintf(strval, "S%d (unknown)", res[1]);
			break;
	}
	fjson_object_object_add(root_object, "SystemPowerState", fjson_object_new_string(strval));

	sprintf(strval, "%d", res[3]&0x7);
	fjson_object_object_add(root_object, "WatchdogTimerCurrentValue", fjson_object_new_string(strval));

	reg4_object = fjson_object_new_object();
	fjson_object_object_add(reg4_object, "INTRD_DET", fjson_object_new_string((res[4] & (1<<0))?"1":"0"));
	fjson_object_object_add(reg4_object, "BTI", fjson_object_new_string((res[4] & (1<<1))?"1":"0"));
	fjson_object_object_add(reg4_object, "DOA", fjson_object_new_string((res[4] & (1<<2))?"1":"0"));
	fjson_object_object_add(reg4_object, "SECOND_TO_STS", fjson_object_new_string((res[4] & (1<<3))?"1":"0"));
	fjson_object_object_add(reg4_object, "SMBALERT", fjson_object_new_string((res[4] & (1<<7))?"1":"0"));
	fjson_object_object_add(root_object, "reg4", reg4_object);

	reg5_object = fjson_object_new_object();
	fjson_object_object_add(reg5_object, "FWH", fjson_object_new_string((res[4] & (1<<0))?"1":"0"));
	fjson_object_object_add(reg5_object, "SYS_PWROK_Failure", fjson_object_new_string((res[4] & (1<<2))?"1":"0"));
	fjson_object_object_add(reg5_object, "POWER_OK_BAD", fjson_object_new_string((res[4] & (1<<5))?"1":"0"));
	fjson_object_object_add(reg5_object, "ThermalTrip", fjson_object_new_string((res[4] & (1<<6))?"1":"0"));
	fjson_object_object_add(root_object, "reg5", reg5_object);

	sprintf(strval, "%d", res[6]);
	fjson_object_object_add(root_object, "Message1", fjson_object_new_string(strval));

	sprintf(strval, "%d", res[7]);
	fjson_object_object_add(root_object, "Message2", fjson_object_new_string(strval));

	sprintf(strval, "%d", res[8]);
	fjson_object_object_add(root_object, "TCO_WDCNT", fjson_object_new_string(strval));


	sprintf(strval, "%02x", res[9]);
	fjson_object_object_add(root_object, "Seconds", fjson_object_new_string(strval));

	sprintf(strval, "%02x", res[10]);
	fjson_object_object_add(root_object, "Minutes", fjson_object_new_string(strval));

	sprintf(strval, "%02x", res[11]);
	fjson_object_object_add(root_object, "Hours", fjson_object_new_string(strval));

	sprintf(strval, "%x", res[12]);
	fjson_object_object_add(root_object, "DayOfWeek", fjson_object_new_string(strval));

	sprintf(strval, "%x", res[13]);
	fjson_object_object_add(root_object, "DayOfMonth", fjson_object_new_string(strval));

	sprintf(strval, "%x", res[14]);
	fjson_object_object_add(root_object, "Month", fjson_object_new_string(strval));

	sprintf(strval, "%x", res[15]);
	fjson_object_object_add(root_object, "Year", fjson_object_new_string(strval));


	printf("%s\n", fjson_object_to_json_string(root_object));

	fjson_object_put(root_object);

	closelog();

	return 0;
}

