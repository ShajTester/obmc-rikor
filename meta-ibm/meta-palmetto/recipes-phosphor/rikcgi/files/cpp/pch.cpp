
#define _GNU_SOURCE

#include <iostream>
#include <fstream>
#include <string>
#include <array>

#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "smbus.h"

#include "json.hpp"
using json = nlohmann::json;


#define sizeof_sensors_data (sizeof(sensors_data) / sizeof(sensor))


int main(int argc, char const *argv[])
{
    openlog("rikcgi-pch", LOG_CONS, LOG_USER);
    // syslog(LOG_INFO, "rikcgi-voltagesensors: started");
	
	int file;
	int adapter_nr = 3; /* probably dynamically determined */
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
	file = open(filename, O_RDWR);
	if (file < 0) {
		/* ERROR HANDLING; you can check errno to see what went wrong */
		fprintf(stderr, "Device open error %d\n", errno);
		exit(1);
	}


	int addr = 0x44; /* The I2C address */

	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		/* ERROR HANDLING; you can check errno to see what went wrong */
		fprintf(stderr, "Add slave error %d\n", errno);
		exit(1);
	}


	std::array<__u8, 16> res;

	/* Using SMBus commands */
	for(int i=0; i<16; i++)
	{
		res[i] = i2c_smbus_read_byte_data(file, i);
		if (res[i] < 0) {
			/* ERROR HANDLING: i2c transaction failed */
			fprintf(stderr, "Read register error %d\n", res[i]);
		}
	}

	json jout;

	char strval[80];

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
	jout["SystemPowerState"] = std::string(strval);

	sprintf(strval, "%d", res[3]&0x7);
	jout["WatchdogTimerCurrentValue"] =  std::string(strval);

	jout["reg4"] = json::object();
	jout["reg4"]["INTRD_DET"] = (res[4] & (1<<0))?"1":"0";
	jout["reg4"]["BTI"] = (res[4] & (1<<1))?"1":"0";
	jout["reg4"]["DOA"] = (res[4] & (1<<2))?"1":"0";
	jout["reg4"]["SECOND_TO_STS"] = (res[4] & (1<<3))?"1":"0";
	jout["reg4"]["SMBALERT"] = (res[4] & (1<<7))?"1":"0";

	jout["reg5"] = json::object();
	jout["reg5"]["FWH"] = (res[5] & (1<<0))?"1":"0";
	jout["reg5"]["SYS_PWROK_Failure"] = (res[5] & (1<<2))?"1":"0";
	jout["reg5"]["POWER_OK_BAD"] = (res[5] & (1<<5))?"1":"0";
	jout["reg5"]["ThermalTrip"] = (res[5] & (1<<6))?"1":"0";

	sprintf(strval, "%d", res[6]);
	jout["Message1"] =  std::string(strval);

	sprintf(strval, "%d", res[7]);
	jout["Message2"] =  std::string(strval);

	sprintf(strval, "%d", res[8]);
	jout["TCO_WDCNT"] =  std::string(strval);


	sprintf(strval, "%02x", res[9]);
	jout["Seconds"] =  std::string(strval);

	sprintf(strval, "%02x", res[10]);
	jout["Minutes"] =  std::string(strval);

	sprintf(strval, "%02x", res[11]);
	jout["Hours"] =  std::string(strval);

	sprintf(strval, "%x", res[12]);
	jout["DayOfWeek"] =  std::string(strval);

	sprintf(strval, "%x", res[13]);
	jout["DayOfMonth"] =  std::string(strval);

	sprintf(strval, "%x", res[14]);
	jout["Month"] =  std::string(strval);

	sprintf(strval, "%x", res[15]);
	jout["Year"] =  std::string(strval);



	// json jout(outdata);
	std::cout << jout << std::endl;

	closelog();

	return 0;
}


