
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


/// gDBus
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "rikbtnd-manager.h"
#include "identify-manager.h"


#define POWER_OFF_STR "Off"
#define POWER_ON_STR  "Running"


pthread_mutex_t web_mutex1 = PTHREAD_MUTEX_INITIALIZER;
XyzOpenbmc_projectAresRikbtnd *rikbtnd_interface;

static void gpio_event_handle(gpio_poll_st *gp);

// GPIO table to be monitored when MB is ON
static gpio_poll_st g_gpios[] =
{
    // {{gpio, fd}, edge, gpioValue, call-back function, GPIO description}
    {{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOD5", "FP_ID_BTN" },
    {{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOR7", "FP_PWR_BTN_MUX_N"},
    {{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOR6", "FM_BMC_PWR_BTN_N"},
    {{0, 0}, GPIO_EDGE_FALLING, 0, gpio_event_handle, "GPIOR4", "FP_RST_BTN_N"},
};

static int g_count = sizeof(g_gpios) / sizeof(gpio_poll_st);


bool power_state;
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
        usleep(100000);
        gpio_set(g.gs_gpio, GPIO_VALUE_LOW);
        usleep(100000);
        gpio_set(g.gs_gpio, GPIO_VALUE_HIGH);
        gpio_change_direction(&g, GPIO_DIRECTION_IN);

        system("/usr/bin/rikbtnd-afterpoweron.sh &");

        sprintf(blink_cmd, "/usr/bin/ledblink-1.0 %d 10 &", gpio_num("GPIOQ7"));
        // syslog(LOG_INFO, blink_cmd);
        system(blink_cmd);
    }

    if (rikbtnd_interface != NULL)
        xyz_openbmc_project_ares_rikbtnd_set_host_power_mode(rikbtnd_interface, power_state ? POWER_OFF_STR : POWER_ON_STR);

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

    // sleep(2);    // Попытка решения проблемы #100

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


void on_id_pressed_hundler()
{
    XyzOpenbmc_projectLedPhysical *proxy;
    GError *error;
    const gchar *response;

    syslog(LOG_INFO, "ID button pressed");

    error = NULL;
    proxy = xyz_openbmc_project_led_physical_proxy_new_for_bus_sync (
                G_BUS_TYPE_SYSTEM,
                G_DBUS_PROXY_FLAGS_NONE,
                "xyz.openbmc_project.LED.Controller.identify",    /* bus name */
                "/xyz/openbmc_project/led/physical/identify",     /* object */
                NULL,                                             /* GCancellable* */
                &error);
    /* do stuff with proxy */

    if (error != NULL)
    {
        syslog(LOG_ERR, "ERROR %s", error->message);
    }

    if (proxy != NULL)
    {
        response = xyz_openbmc_project_led_physical_get_state(proxy);
        syslog(LOG_INFO, "response is \"%s\"", response);
        if (g_strcmp0(response, "xyz.openbmc_project.Led.Physical.Action.Off") == 0)
        {
            xyz_openbmc_project_led_physical_set_state(proxy, "xyz.openbmc_project.Led.Physical.Action.Blink");
        }
        else
        {
            xyz_openbmc_project_led_physical_set_state(proxy, "xyz.openbmc_project.Led.Physical.Action.Off");
        }
    }
    else
    {
        syslog(LOG_ERR, "proxy == NULL");
    }

    // g_free(response);
    g_object_unref (proxy);
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
    {
        // Front panel ID button
        tt = get_nanos();
        if ((tt - id_last_time) > 600)
        {
            // if (gpio_get(g_gpios[0].gs.gs_gpio) == GPIO_VALUE_LOW)
            // {
            // 	syslog(LOG_INFO, "ID button pressed");
            // 	gpio_set(gpio_num("GPIOD6"), GPIO_VALUE_HIGH);
            // }

            on_id_pressed_hundler();

        }
        id_last_time = tt;
    }
    else if (gp->gs.gs_gpio == g_gpios[1].gs.gs_gpio)
    {
        // Front panel POWER button
        if (!PCH_cmd_flag)
        {
            // Нет подачи сигнала на PCH
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
    {
        // Front panel RESET button
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





static gboolean on_handle_host_power (XyzOpenbmc_projectAresRikbtnd *interface,
                                      GDBusMethodInvocation *invocation,
                                      const gchar *greeting, gpointer user_data)
{
    gchar *response;

    power_command();
    PCH_command();

    response = g_strdup(power_state ? POWER_OFF_STR : POWER_ON_STR);
    xyz_openbmc_project_ares_rikbtnd_set_host_power_mode(interface, response);
    xyz_openbmc_project_ares_rikbtnd_complete_host_power(interface, invocation, response);
    g_free(response);

    return TRUE;
}



static void on_bus_acquired (GDBusConnection *connection,
                             const gchar     *name,
                             gpointer         user_data)
{
    XyzOpenbmc_projectAresRikbtnd *interface;
    GError *error;

    /* This is where we'd export some objects on the bus */
    syslog(LOG_INFO, "on_bus_acquired %s\n", name);


    gchar *conn_name;
    g_object_get(connection, "unique-name", &conn_name, NULL);
    syslog(LOG_INFO, "%s\n", conn_name);
    g_free(conn_name);

    interface = xyz_openbmc_project_ares_rikbtnd_skeleton_new();
    g_signal_connect (interface, "handle-host-power", G_CALLBACK (on_handle_host_power), NULL);
    // g_signal_connect (interface, "host-power-mode", G_CALLBACK (on_handle_host_power_mode), NULL);
    error = NULL;
    if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface), connection, "/xyz/openbmc_project/ares/rikbtnd", &error))
    {
        g_print("ERROR %s\n", error->message);
        rikbtnd_interface = NULL;
    }
    else
    {
        rikbtnd_interface = interface;
    }
    xyz_openbmc_project_ares_rikbtnd_set_host_power_mode(interface, POWER_OFF_STR);
}


static void on_name_lost (GDBusConnection *connection,
                          const gchar     *name,
                          gpointer         user_data)
{
    syslog(LOG_ERR, "on_name_lost %s on the session bus\n", name);
}


static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    // syslog(LOG_INFO, "on_name_acquired %s on the session bus\n", name);
}










void gpio_poll_thread(void *arg)
{
    gpio_poll_open(g_gpios, g_count);
    gpio_poll(g_gpios, g_count, -1);
}


static 	GMainLoop *loop;


/*
 * On SIGINT, exit the main loop
 */
static void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        g_main_loop_quit(loop);
    }
}

// static const char pidfilename[] = "/var/run/rikbtnd.pid";

int main(int argc, char **argv)
{
    // int rc;
    // int pid_file;

    rikbtnd_interface = NULL;
    pthread_t cmd_thread;
    // // pthread_t tim_thread;

    openlog("rikbtnd", LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "rikbtnd daemon started. ver 0.6");

    sleep(10);

    gpio_set(gpio_num("GPIOQ7"), GPIO_VALUE_LOW);

    PCH_cmd_flag = false;

    // set up the SIGINT signal handler
    if (signal(SIGINT, &sig_handler) == SIG_ERR)
    {
        syslog(LOG_INFO, "Failed to register SIGINT handler, quitting...\n");
        exit(EXIT_FAILURE);
    }

    pthread_create(&cmd_thread, NULL, &gpio_poll_thread, NULL);

    loop = g_main_loop_new (NULL, FALSE);

    // guint bus_id = g_bus_own_name(G_BUS_TYPE_SESSION, "com.rikor", G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
    //                on_name_acquired, on_name_lost, NULL, NULL);
    guint bus_id = g_bus_own_name(G_BUS_TYPE_SYSTEM, "xyz.openbmc_project.ares.rikbtnd",
                                  G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
                                  on_name_acquired, on_name_lost, NULL, NULL);

    syslog(LOG_INFO, "Initial PID: %d\n", getpid());

    g_main_loop_run (loop);
    g_main_loop_unref(loop);
    g_bus_unown_name(bus_id);

    syslog(LOG_ERR, "rikbtnd closed ...");
    gpio_poll_close(g_gpios, g_count);

    return 0;
}
