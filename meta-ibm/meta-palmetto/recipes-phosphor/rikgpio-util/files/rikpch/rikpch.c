

#define _GNU_SOURCE

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

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

// #define DEBUG_MODE


static const __u8 regs[]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

int main(void)
{
	// int tmp = 0;
	int i;

	printf("Well, let's start rikpch...\n\n\n");

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


	// __u8 reg = 0x10; /* Device register to access */
	__u8 res;
	// char buf[10];

	/* Using SMBus commands */
	for(i=0; i<sizeof(regs); i++)
	{
		res = i2c_smbus_read_byte_data(file, regs[i]);
		if (res < 0) {
			/* ERROR HANDLING: i2c transaction failed */
			fprintf(stderr, "Read register error %d\n", res);
		} else {
			/* res contains the read word */
			printf("Register 0x%02X  value 0x%02X\n", regs[i], res);
		}
	}

	printf("\nInterpretation of values:\n\n");

	res = i2c_smbus_read_byte_data(file, 1);
	if (res < 0) {
		fprintf(stderr, "Read register 1 error %d\n", res);
	} else {
		switch(res) {
			case 0:
			case 1:
			case 3:
			case 4:
			case 5:
				printf("System Power state S%d\n", res);
				break;

			default:
				printf("System Power state in RESERVED STATE\n");
				break;
		}
	}

	res = i2c_smbus_read_byte_data(file, 3);
	if (res < 0) {
		fprintf(stderr, "Read register 3 error %d\n", res);
	} else {
		printf("Watchdog Timer current value %d\n", res);
	}

	res = i2c_smbus_read_byte_data(file, 4);
	if (res < 0) {
		fprintf(stderr, "Read register 4 error %d\n", res);
	} else {
		printf("\nRegister 4 bits:\n");
		printf("0 The Intruder Detect (INTRD_DET) bit is %s\n", (res & (1<<0))?"set":"clr");
		printf("1 BTI Temperature Event %s\n", (res & (1<<1))?"occured":"clr");
		printf("2 DOA Processor Status %s\n", (res & (1<<2))?"dead":"clr");
		printf("3 SECOND_TO_STS bit %s\n", (res & (1<<3))?"set":"clr");
		printf("7 SMBALERT# pin %s\n", (res & (1<<7))?"set":"clr");
	}

	res = i2c_smbus_read_byte_data(file, 5);
	if (res < 0) {
		fprintf(stderr, "Read register 5 error %d\n", res);
	} else {
		printf("\nRegister 5 bits:\n");
		printf("0 FWH bad bit %s\n", (res & (1<<0))?"set":"clr");
		printf("2 SYS_PWROK Failure Status %s\n", (res & (1<<2))?"set":"clr");
		printf("5 POWER_OK_BAD bit %s\n", (res & (1<<5))?"set":"clr");
		printf("6 Thermal Trip bit %s\n\n", (res & (1<<6))?"set":"clr");
	}

	res = i2c_smbus_read_byte_data(file, 6);
	if (res < 0) {
		fprintf(stderr, "Read register 6 error %d\n", res);
	} else {
		printf("Message 1 register value 0x%02X\n", res);
	}

	res = i2c_smbus_read_byte_data(file, 7);
	if (res < 0) {
		fprintf(stderr, "Read register 7 error %d\n", res);
	} else {
		printf("Message 2 register value 0x%02X\n", res);
	}

	res = i2c_smbus_read_byte_data(file, 8);
	if (res < 0) {
		fprintf(stderr, "Read register 8 error %d\n", res);
	} else {
		printf("TCO_WDCNT register value 0x%02X\n", res);
	}

	__u8 RTC[7];

	printf("\n|  6  |  5  |  4  |  3  |  2  |  1  |  0  |\n");
	while(1)
	{
		for(i=0; i<7; i++)	{
			RTC[i] = i2c_smbus_read_byte_data(file, i+9);
			if (RTC[i] < 0) {
				fprintf(stderr, "Read register %d error %d\n", i+9, RTC[i]);
			}
		}
		printf("| %3d | %3d | %3d | %3d | %3d | %3d | %3d |", RTC[6], RTC[5], RTC[4], RTC[3], RTC[2], RTC[1], RTC[0]);
		printf(" --- | %02x | %02x | %02x | %02x | %02x | %02x | %02x |\n", RTC[6], RTC[5], RTC[4], RTC[3], RTC[2], RTC[1], RTC[0]);
		usleep(500000);
	}

	printf("\n...rikpch Complete.\n");

	return 0;
}

