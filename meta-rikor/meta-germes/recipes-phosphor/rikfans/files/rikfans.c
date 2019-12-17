//#include <stdio.h>
//#include <stdint.h>
//#include <stdlib.h>
//#include <string.h>
//#include <iostream>
//#include <fstream>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define DEBUG

#define SENSE_RAW_PATH  "/sys/class/i2c-dev/i2c-6/device/6-00%s/hwmon/hwmon%d/temp1_input"
#define PWM_TACH_PATH "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm%d"

/************************************************************************************************************************************************************/
//Primitives
unsigned int getTemperature(int num);		//get sensor temperature value
void setPWM(unsigned int coef, int chNum);	//set PWM coefficient

/************************************************************************************************************************************************************/

static void strip(char *str) {
  while(*str != '\0') {
    if (!isalnum(*str)) {
      *str = '\0';
      break;
    }
    str++;
  }
}

/************************************************************************************************************************************************************/
int main (void)
{
	unsigned int ctemp4f = 0;
	unsigned int ctemp4e = 0;
	unsigned int ctemp4c = 0;
	unsigned int ctemp49 = 0;
	unsigned int ctemp4b = 0;
	
	unsigned int tmpVal0 = 0;
	unsigned int tmpVal1 = 0;
	unsigned int tmpVal2 = 0;
	
	ctemp4f = getTemperature(0);
	ctemp4e = getTemperature(1);
	ctemp4c = getTemperature(2);
	ctemp49 = getTemperature(3);
	ctemp4b = getTemperature(4);
	
	tmpVal0 = (ctemp4f+ctemp4e+ctemp4c)/3;
	tmpVal1 = ctemp49;
	tmpVal2 = ctemp4b;
	
	if (tmpVal0 > 60)
	{
		//Поднять ШИМ на соответствующих каналах ШИМ
		setPWM(110,0);
	}
	else
	{
		setPWM(50,0);
	}
	if (tmpVal1 > 60)
	{
		//Поднять ШИМ на соответствующих каналах ШИМ
		setPWM(110,1);
	}
	else
	{
		setPWM(50,1);
	}
	if (tmpVal2 > 60)
	{
		//Поднять ШИМ на соответствующих каналах ШИМ
		setPWM(110,2);
	}
	else
	{
		setPWM(50,2);
	}
	return 0;
}

/************************************************************************************************************************************************************/
unsigned int getTemperature(int num)
{
	char buf[128] = {0};
  	int fd = -1;
	unsigned int val = 25;

	//Считать цифру с датчика, перевести в int, отнормировать и вернуть в main
	switch (num)
	{
		case 0:
			//Check 4f
			//snprintf(tVal, SENSE_RAW_PATH, "4f", 5);
			snprintf(buf, sizeof(buf), SENSE_RAW_PATH, "4f", 5);
			fd = open(buf, O_RDONLY);
			read(fd, buf, sizeof(buf));
  			strip(buf);
			close (fd);
			val = atoi(buf);
			val = val / 1000;
			break;
		case 1:
			//Check 4e
			//snprintf(tVal, SENSE_RAW_PATH, "4e", 2);
			snprintf(buf, sizeof(buf), SENSE_RAW_PATH, "4e", 2);
			fd = open(buf, O_RDONLY);
			read(fd, buf, sizeof(buf));
  			strip(buf);
			close (fd);
			val = atoi(buf);
			val = val / 1000;
			break;
		case 2:
			//Check 4c
			//snprintf(tVal, SENSE_RAW_PATH, "4c", 3);
			snprintf(buf, sizeof(buf), SENSE_RAW_PATH, "4c", 3);
			fd = open(buf, O_RDONLY);
			read(fd, buf, sizeof(buf));
  			strip(buf);
			close (fd);
			val = atoi(buf);
			val = val / 1000;
			break;
		case 3:
			//Check 49
			//snprintf(tVal, SENSE_RAW_PATH, "49", 1);
			snprintf(buf, sizeof(buf), SENSE_RAW_PATH, "49", 1);
			fd = open(buf, O_RDONLY);
			read(fd, buf, sizeof(buf));
  			strip(buf);
			close (fd);
			val = atoi(buf);
			val = val / 1000;
			break;
		case 4:
			//Check 4b
			//snprintf(tVal, SENSE_RAW_PATH, "4b", 4);
			snprintf(buf, sizeof(buf), SENSE_RAW_PATH, "4b", 4);
			fd = open(buf, O_RDONLY);
			read(fd, buf, sizeof(buf));
  			strip(buf);
			close (fd);
			val = atoi(buf);
			val = val / 1000;
			break;
		default:
			break;
	}
	
	#ifdef DEBUG
	printf ("DEBUG_rikfans_%d: %d\n", (int)num, (unsigned int)val);
	#endif		//DEBUG
	
	return val;
}

/************************************************************************************************************************************************************/
void setPWM(unsigned int coef, int chNum)
{
	char *c;
	char buf[128];
  	int fd = -1;
	
	if (coef == 110) c = "110";
	else c = "50";
	
	switch (chNum)
	{
		case 0:
			for (int i=1; i<4 ;i++)
			{
				snprintf(buf, sizeof(buf), PWM_TACH_PATH, i);
  				fd = open(buf, O_WRONLY);
  				write(fd, c, strlen(c));
				close(fd);	
  			}
			break;
		case 1:
				snprintf(buf, sizeof(buf), PWM_TACH_PATH, 5);
  				fd = open(buf, O_WRONLY);
  				write(fd, c, strlen(c));
				close(fd);	
			break;
		case 2:
				snprintf(buf, sizeof(buf), PWM_TACH_PATH, 6);
  				fd = open(buf, O_WRONLY);
  				write(fd, c, strlen(c));
				close(fd);	
			break;
		default:
			break;
	}
}



