#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <dirent.h>

#include <argp.h>
#include <sys/stat.h>

#include <sys/ioctl.h>

void get_sensors_presence(int number);
void get_eeprom_presence();
void print_target_table();

static void strip(char *str) {
  while(*str != '\0') {
    if (!isalnum(*str)) {
      *str = '\0';
      break;
    }
    str++;
  }
}

/********************************************************************************************/
int main(int argc, char const *argv[])
{
	int num = 0;
	for (num=0; num<12; num++)
	{
		get_sensors_presence(num);
	}
	get_eeprom_presence();
	print_target_table();
	return 0;
}

/********************************************************************************************/
void get_sensors_presence(int number)
{
  	char buf[128] = {0};
	char data[128] = {0};
  	int fd = -1;

	snprintf(buf, sizeof(buf), "/sys/class/hwmon/hwmon%d/name", number);
  	fd = open(buf, O_RDONLY);
  	if (fd == -1) {
    		return;
  	}

  	read(fd, data, sizeof(data));
	//printf("%s\n detected",data);
  	strip(data);
	if (!strcmp(data, "aspeed_pwm_tacho")) {
    		printf("FANs sensors detected \n");
  	} else if (!strcmp(data, "lm75")) {
    		printf("lm75 sensor detected \n");
  	} else if (!strcmp(data, "tmp421")) {
    		printf("tmp421 sensor detected \n");
  	} else if (!strcmp(data, "pmbus")) {
    		printf("pmbus sensor detected \n");
  	} else if (!strcmp(data, "iio_hwmon")) {
    		printf("ADC sensor detected \n");
  	} else if (!strcmp(data, "peci_cputemp.cpu0")) {
    		printf("CPU0 temp sensor detected \n");
  	} else if (!strcmp(data, "peci_dimmtemp.cpu0")) {
    		printf("CPU0 DIMMs temp sensors detected \n");
  	} else if (!strcmp(data, "peci_cputemp.cpu1")) {
    		printf("CPU1 temp sensor detected \n");
  	} else if (!strcmp(data, "peci_dimmtemp.cpu1")) {
    		printf("CPU1 DIMMs temp sensors detected \n");
  	} else {
    		printf("Nothing \n");
  	}
  	close(fd);
  	return;
}

/********************************************************************************************/
void get_eeprom_presence()
{
	char buf[128] = {0};
	char data[128] = {0};
  	int fd = -1;

	snprintf(buf, sizeof(buf), "/sys/class/i2c-dev/i2c-3/device/3-0056/name");
  	fd = open(buf, O_RDONLY);
  	if (fd == -1) {
    		return;
  	}

  	read(fd, data, sizeof(data));
	//printf("%s\n detected",data);
  	strip(buf);
	if (!strcmp(data, "24c02")) {
    		printf("AT24C02 FRU EEPROM detected \n");
  	} else {
    		printf("Nothing \n");
  	}
  	close(fd);
  	return;
}

/********************************************************************************************/
void print_target_table()
{
	printf("\nTARGET:\n");
    	printf("FANs sensors detected \n");
    	printf("lm75 sensor detected \n");
	printf("lm75 sensor detected \n");
	printf("lm75 sensor detected \n");
	printf("lm75 sensor detected \n");
	printf("tmp421 sensor detected \n");
	printf("pmbus sensor detected \n");
	printf("pmbus sensor detected \n");
	printf("ADC sensor detected \n");
	printf("CPU0 temp sensor detected \n");
	printf("CPU0 DIMMs temp sensors detected \n");
	printf("CPU1 temp sensor detected \n");
	printf("CPU1 DIMMs temp sensors detected \n");
	printf("AT24C02 FRU EEPROM detected \n");
}
