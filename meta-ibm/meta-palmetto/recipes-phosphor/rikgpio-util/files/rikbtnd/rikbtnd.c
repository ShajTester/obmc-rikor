/*
 * sensord
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#define _GNU_SOURCE

#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <syslog.h>

#include <math.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/file.h>

#include <dirent.h>

#include <sys/ioctl.h>
// #include <linux/gpio.h>

#include <openbmc/gpio.h>

int get_gpio_base();
int gpio_num(char *str);

pthread_mutex_t web_mutex1 = PTHREAD_MUTEX_INITIALIZER;

int gpio_base = -1;

int power_command()
{
	gpio_st g;
	g.gs_gpio = gpio_num("GPIOY3") + gpio_base;
	syslog(LOG_INFO, "gpio_num(\"GPIOY3\") is %d", gpio_num("GPIOY3"));
	syslog(LOG_INFO, "gpio_base is %d", gpio_base);
	syslog(LOG_INFO, "GPIOY3 number is %d", g.gs_gpio);

	pthread_mutex_lock(&web_mutex1);

	gpio_change_direction(&g, GPIO_DIRECTION_OUT);
	gpio_set(g.gs_gpio, GPIO_VALUE_HIGH);
	gpio_set(g.gs_gpio, GPIO_VALUE_LOW);
	gpio_change_direction(&g, GPIO_DIRECTION_IN);

	pthread_mutex_unlock(&web_mutex1);
	return 0;
}


bool power_state;

bool get_power_state()
{

	return false;
}

int power_state_store()
{

	return 0;
}

// Отсюда
// https://stackoverflow.com/a/36095407
static long long get_nanos(void)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return (long long)ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

// static long long last_nanos = 0;
static long long id_last_time = 0;
static long long pwrbtn_last_time = 0;

// Generic Event Handler for GPIO changes
static void gpio_event_handle(gpio_poll_st *gp)
{
	// char cmd[128] = {0};
	long long tt;

	if (gp->gs.gs_gpio == gpio_num("GPIOD5") + gpio_base)
	{	// Front panel ID button
		tt = get_nanos();
		if ((tt - id_last_time) > 600)
		{
			if (gpio_get(gpio_num("GPIOD5") + gpio_base) == GPIO_VALUE_LOW)
			{
				syslog(LOG_INFO, "ID button pressed");
				gpio_set(gpio_num("GPIOD6") + gpio_base, GPIO_VALUE_HIGH);
			}
		}
		id_last_time = tt;
	}
	else if (gp->gs.gs_gpio == gpio_num("GPIOR7") + gpio_base)
	{	// Front panel POWER button
		tt = get_nanos();
		if ((tt - pwrbtn_last_time) > 600)
		{
			if (gpio_get(gpio_num("GPIOR7") + gpio_base) == GPIO_VALUE_LOW)
			{
				syslog(LOG_INFO, "POWER button pressed");
				power_state ^= true;
				syslog(LOG_INFO, "POWER state is %d", power_state);
				power_state_store();
			}
		}
		pwrbtn_last_time = tt;
	}
	// else if (gp->gs.gs_gpio == gpio_num("GPIOL0") + gpio_base) { // IRQ_UV_DETECT_N
	//   log_gpio_change(gp, 20*1000);
	// }
	// long long nanos = get_nanos();
	// syslog(LOG_CRIT, "delta %10lld value %d: %s - %s\n", nanos - last_nanos, gp->value, gp->name, gp->desc);
	// last_nanos = nanos;
}



// GPIO table to be monitored when MB is ON
static gpio_poll_st g_gpios[] = {
	// {{gpio, fd}, edge, gpioValue, call-back function, GPIO description}
	{{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOD5", "FP_ID_BTN" },
	{{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOR7", "FP_PWR_BTN_MUX_N"},
	{{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOR6", "FM_BMC_PWR_BTN_N"},
};

static int g_count = sizeof(g_gpios) / sizeof(gpio_poll_st);


void *start_pipe(void *ptr)
{
	int fd1;
	int rc;

	// FIFO file path
	char * myfifo = "/tmp/rikbtnd.pipe";

	// Creating the named file(FIFO)
	// mkfifo(<pathname>,<permission>)
	mkfifo(myfifo, 0666);

	char str1[81];
	// char str2[81];
	while (1)
	{
		// First open in read only and read
		fd1 = open(myfifo, O_RDONLY);
		rc = read(fd1, str1, 80);
		if(rc == -1)
		{
			syslog(LOG_ERR, "Read pipe error");
		}
		else if(rc == 0)
		{
		}
		else
		{
			str1[rc] = 0;
			syslog(LOG_INFO, "Read string from pipe: %s", str1);
			if(strcmp(str1, "switch power"))
			{
				power_command();
				power_state ^= true;
				syslog(LOG_INFO, "POWER state is %d", power_state);
				power_state_store();
			}
		}
		close(fd1);

		// Now open in write mode and write
		// string taken from user.
		fd1 = open(myfifo,O_WRONLY);
		if(power_state)
			write(fd1, "on\0", 3);
		else
			write(fd1, "off\0", 4);
		close(fd1);
	}

	unlink(myfifo);
}


static const char pidfilename[] = "/var/run/rikbtnd.pid";

int
main(int argc, char **argv)
{
	int rc;
	int pid_file;

	pthread_t web_thread;

	openlog("rikbtnd", LOG_CONS, LOG_DAEMON);

	pid_file = open(pidfilename, O_CREAT | O_RDWR, 0666);
	rc = flock(pid_file, LOCK_EX | LOCK_NB);
	if (rc) 
	{
		if (EWOULDBLOCK == errno) 
		{
			syslog(LOG_ERR, "Another rikbtnd instance is running...");
			exit(-1);
		}
	} 
	else 
	{
		daemon(0, 1);

		char tstr[32];
		sprintf(tstr, "%d", getpid());
		printf("rikbtnd daemon started. ver 0.4. PID %s", tstr);
		write(pid_file, tstr, strlen(tstr));

		gpio_base = get_gpio_base();
		
		power_state = get_power_state();
		if(power_state)
			power_command();

		rc = pthread_create(&web_thread, NULL, start_pipe, NULL);
		if (rc)
			syslog(LOG_ERR, "pthread_create error.");

		gpio_poll_open(g_gpios, g_count);
		gpio_poll(g_gpios, g_count, -1);
		gpio_poll_close(g_gpios, g_count);

		pthread_join(web_thread, NULL);
	}

	unlink(pidfilename);
    flock(pid_file, LOCK_UN);

	return 0;
}





/**
 * Determine the GPIO base number for the system.  It is found in
 * the 'base' file in the /sys/class/gpio/gpiochipX/ directory where the
 * /sys/class/gpio/gpiochipX/label file has a '1e780000.gpio' in it.
 *
 * Note: This method is ASPEED specific.  Could add support for
 * additional SOCs in the future.
 *
 * @return int - the GPIO base number, or < 0 if not found
 */

#define GPIO_PORT_OFFSET 8
#define GPIO_BASE_PATH "/sys/class/gpio"


int get_gpio_base()
{
  int gpio_base = -1;

  DIR* dir = opendir(GPIO_BASE_PATH);
  if (dir == NULL)
  {
    fprintf(stderr, "Unable to open directory %s\n",
        GPIO_BASE_PATH);
    return -1;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != NULL)
  {
    /* Look in the gpiochip<X> directories for a file called 'label' */
    /* that contains '1e780000.gpio', then in that directory read */
    /* the GPIO base out of the 'base' file. */

    if (strncmp(entry->d_name, "gpiochip", 8) != 0)
    {
      continue;
    }

    //gboolean is_bmc = FALSE;
    int is_bmc = 0;
    char* label_name;
    asprintf(&label_name, "%s/%s/label",
        GPIO_BASE_PATH, entry->d_name);

    FILE* fd = fopen(label_name, "r");
    free(label_name);

    if (!fd)
    {
      continue;
    }

    char label[14];
    if (fgets(label, 14, fd) != NULL)
    {
      if (strcmp(label, "1e780000.gpio") == 0)
      {
        //is_bmc = TRUE;
	is_bmc = 1;
      }
    }
    fclose(fd);

    if (!is_bmc)
    {
      continue;
    }

    char* base_name;
    asprintf(&base_name, "%s/%s/base",
        GPIO_BASE_PATH, entry->d_name);

    fd = fopen(base_name, "r");
    free(base_name);

    if (!fd)
    {
      continue;
    }

    if (fscanf(fd, "%d", &gpio_base) != 1)
    {
      gpio_base = -1;
    }
    fclose(fd);

    /* We found the right file. No need to continue. */
    break;
  }
  closedir(dir);

  if (gpio_base == -1)
  {
    fprintf(stderr, "Could not find GPIO base\n");
  }

  return gpio_base;
}



#define GPIO_BASE_AA0  208


int gpio_num(char *str)
{
  int len = strlen(str);
  int ret = 0;

  if (len != 6 && len != 7) {
    return -1;
  }
  ret = str[len-1] - '0' + (8 * (str[len-2] - 'A'));
  if (len == 7)
    ret += GPIO_BASE_AA0;
  return ret;
}


