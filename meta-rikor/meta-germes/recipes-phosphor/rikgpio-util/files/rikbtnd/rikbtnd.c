
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


pthread_mutex_t web_mutex1 = PTHREAD_MUTEX_INITIALIZER;

static void gpio_event_handle(gpio_poll_st *gp);

// GPIO table to be monitored when MB is ON
static gpio_poll_st g_gpios[] = {
	// {{gpio, fd}, edge, gpioValue, call-back function, GPIO description}
	{{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOD5", "FP_ID_BTN" },
	{{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOR7", "FP_PWR_BTN_MUX_N"},
	{{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOR6", "FM_BMC_PWR_BTN_N"},
	{{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOR4", "FP_RST_BTN_N"},
};

static int g_count = sizeof(g_gpios) / sizeof(gpio_poll_st);


// bool power_state;
bool PCH_cmd_flag;


bool get_power_state()
{
	return false;
}

int power_state_store()
{
	return 0;
}


#define adc_file_name "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"

int power_command()
{
	gpio_st g;
	char blink_cmd[81];
	FILE *adc_file;
	int adc_val;
	bool power_state;

	g.gs_gpio = gpio_num("GPIOY3");
	// syslog(LOG_INFO, "GPIOY3 number is %d", g.gs_gpio);

	pthread_mutex_lock(&web_mutex1);

	adc_file = fopen(adc_file_name, "r");
	if (adc_file)
	{
		fgets(blink_cmd, sizeof(blink_cmd), adc_file);
		fclose(adc_file);
		adc_val = atoi(blink_cmd);
	}
	else adc_val = 0;

	// if(adc_val > 700) power_state = true;
	// else power_state = false;

	power_state = (adc_val > 700);

	syslog(LOG_INFO, "Power command in power state %d P3V3_BASE raw val %d", power_state, adc_val);

	if (!power_state)
	{
		gpio_change_direction(&g, GPIO_DIRECTION_OUT);
		gpio_set(g.gs_gpio, GPIO_VALUE_HIGH);
		// usleep(10000);
		gpio_set(g.gs_gpio, GPIO_VALUE_LOW);
		gpio_change_direction(&g, GPIO_DIRECTION_IN);

		sprintf(blink_cmd, "/usr/bin/ledblink-1.0 %d 10 &", gpio_num("GPIOQ7"));
		// syslog(LOG_INFO, blink_cmd);
		system(blink_cmd);
	}

	pthread_mutex_unlock(&web_mutex1);
	return 0;
}

int PCH_command()
{
	gpio_st g;
	g.gs_gpio = gpio_num("GPIOD3");

	syslog(LOG_INFO, "PCH command");

	// Поскольку сигналы D3 и R7 внешне связаны,
	// нужно заблокировать срабатывание прерывания по R7

	PCH_cmd_flag = true;

	pthread_mutex_lock(&web_mutex1);

	gpio_change_direction(&g, GPIO_DIRECTION_OUT);
	gpio_set(g.gs_gpio, GPIO_VALUE_LOW);
	usleep(100000);
	gpio_set(g.gs_gpio, GPIO_VALUE_HIGH);
	gpio_change_direction(&g, GPIO_DIRECTION_IN);

	pthread_mutex_unlock(&web_mutex1);

	PCH_cmd_flag = false;

	return 0;
}

int reset_command()
{
	gpio_st g;
	g.gs_gpio = gpio_num("GPIOR5");

	pthread_mutex_lock(&web_mutex1);

	gpio_set(g.gs_gpio, GPIO_VALUE_HIGH);
	gpio_set(g.gs_gpio, GPIO_VALUE_LOW);
	// usleep(500);
	sleep(1);
	gpio_set(g.gs_gpio, GPIO_VALUE_HIGH);

	pthread_mutex_unlock(&web_mutex1);
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
static long long rstbtn_last_time = 0;

// Generic Event Handler for GPIO changes
static void gpio_event_handle(gpio_poll_st *gp)
{
	// char cmd[128] = {0};
	long long tt;

	if (gp->gs.gs_gpio == g_gpios[0].gs.gs_gpio)
	{	// Front panel ID button
		tt = get_nanos();
		if ((tt - id_last_time) > 600)
		{
			if (gpio_get(g_gpios[0].gs.gs_gpio) == GPIO_VALUE_LOW)
			{
				syslog(LOG_INFO, "ID button pressed");
				gpio_set(gpio_num("GPIOD6"), GPIO_VALUE_HIGH);
			}
		}
		id_last_time = tt;
	}
	else if (gp->gs.gs_gpio == g_gpios[1].gs.gs_gpio)
	{	// Front panel POWER button
		if (!PCH_cmd_flag)
		{	// Нет подачи сигнала на PCH
			tt = get_nanos();
			if ((tt - pwrbtn_last_time) > 600)
			{
				if (gpio_get(g_gpios[1].gs.gs_gpio) == GPIO_VALUE_LOW)
				{
					syslog(LOG_INFO, "POWER button pressed");
					power_command();
				}
			}
			pwrbtn_last_time = tt;
		}
	}
	else if (gp->gs.gs_gpio == g_gpios[3].gs.gs_gpio)
	{	// Front panel POWER button
		tt = get_nanos();
		if ((tt - rstbtn_last_time) > 600)
		{
			if (gpio_get(g_gpios[3].gs.gs_gpio) == GPIO_VALUE_LOW)
			{
				syslog(LOG_INFO, "RESET button pressed");
				reset_command();
			}
		}
		rstbtn_last_time = tt;
	}
}


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
		if (rc == -1)
		{
			syslog(LOG_ERR, "Read pipe error");
		}
		else if (rc == 0)
		{
		}
		else
		{
			str1[rc] = 0;
			syslog(LOG_INFO, "Read string from pipe: %s", str1);
			if (strcmp(str1, "switch power"))
			{
				power_command();
				PCH_command();
			}
		}
		close(fd1);

		// Now open in write mode and write
		// string taken from user.
		// fd1 = open(myfifo,O_WRONLY);
		// if(power_state)
		// 	write(fd1, "on\0", 3);
		// else
		// 	write(fd1, "off\0", 4);
		// close(fd1);
	}

	unlink(myfifo);
}


static const char pidfilename[] = "/var/run/rikbtnd.pid";

int main(int argc, char **argv)
{
	int rc;
	int pid_file;

	pthread_t cmd_thread;
	// pthread_t tim_thread;

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
		write(pid_file, tstr, strlen(tstr));
		syslog(LOG_INFO, "rikbtnd daemon started. ver 0.5. PID %s", tstr);

		sleep(10);

		gpio_set(gpio_num("GPIOQ7"), GPIO_VALUE_LOW);

		PCH_cmd_flag = false;

		// power_state = get_power_state();
		// if(power_state)
		// 	power_command();

		rc = pthread_create(&cmd_thread, NULL, start_pipe, NULL);
		if (rc)
			syslog(LOG_ERR, "cmd_thread pthread_create error.");

		// rc = pthread_create(&tim_thread, NULL, flasher_timer, NULL);
		// if (rc)
		// 	syslog(LOG_ERR, "tim_thread pthread_create error.");

		gpio_poll_open(g_gpios, g_count);
		gpio_poll(g_gpios, g_count, -1);
		gpio_poll_close(g_gpios, g_count);

		// pthread_join(tim_thread, NULL);
		pthread_join(cmd_thread, NULL);

		syslog(LOG_ERR, "rikbtnd closed ...");
	}

	unlink(pidfilename);
	flock(pid_file, LOCK_UN);

	return 0;
}
