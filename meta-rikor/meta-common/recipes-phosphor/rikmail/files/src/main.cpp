


#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cmath>
// Отсюда
// https://www.tutorialspoint.com/cplusplus/cpp_signal_handling.htm
#include <csignal>
#include <syslog.h>

#include <vector>
#include <functional>

#include <filesystem>

// #include <unistd.h>
// #include <sys/socket.h>
// #include <sys/un.h>
// #include <sys/stat.h>
// #include <fcntl.h>

#include <nlohmann/json.hpp>

/// gDBus
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "rikmail-manager.h"


namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;
using json = nlohmann::json;

// #define RIKFAN_DEBUG

// gboolean (*handle_smtpparams) (
//   XyzOpenbmc_projectAresRikmail *object,
//   GDBusMethodInvocation *invocation,
//   const gchar *arg_greeting);



static gboolean on_handle_apply_SMTPParams (XyzOpenbmc_projectAresRikmail *interface,
        GDBusMethodInvocation *invocation,
        const gchar           *greeting
        )
// gpointer              user_data)
{
    unsigned int mode = 0;

    syslog(LOG_INFO, "%s", __PRETTY_FUNCTION__);
    syslog(LOG_INFO, "%s", greeting);
    // response = g_strdup(greeting);
    // auto resp = json::array();
    // resp.push_back("test@test.com");
    // resp.push_back(false);
    // resp.push_back(false);
    // resp.push_back(true);
    // resp.push_back(false);

    // https://developer.gnome.org/glib/stable/glib-GVariant.html
    // https://people.gnome.org/~ryanl/glib-docs/gvariant-format-strings.html
    auto mailstr = g_strdup("test@example.org");
    gboolean bool_array[] = {TRUE, FALSE, TRUE, FALSE, TRUE};
    auto response = g_variant_new("(@sab)", &mailstr, &bool_array);
    syslog(LOG_INFO, "%s", mailstr);


    xyz_openbmc_project_ares_rikmail_complete_smtpparams (interface, invocation, response);
    // xyz_openbmc_project_ares_rikmail_complete_smtpparams (interface, invocation, "test@test.com");
    syslog(LOG_INFO, "end function");
    g_free (response);
    g_free (mailstr);

    return TRUE;
}



static void on_bus_acquired (GDBusConnection *connection,
                             const gchar     *name,
                             gpointer         user_data)
{
    XyzOpenbmc_projectAresRikmail *interface;
    GError *error;

    /* This is where we'd export some objects on the bus */
    syslog(LOG_INFO, "on_bus_acquired %s on the session bus\n", name);


    gchar *conn_name;
    g_object_get(connection, "unique-name", &conn_name, NULL);
    syslog(LOG_INFO, "%s\n", conn_name);
    g_free(conn_name);

    interface = xyz_openbmc_project_ares_rikmail_skeleton_new();
    g_signal_connect (interface, "handle-smtpparams",
                      G_CALLBACK (on_handle_apply_SMTPParams),
                      NULL);
    error = NULL;
    if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface), connection, "/xyz/openbmc_project/ares/rikmail", &error))
    {
        g_print("ERROR %s\n", error->message);
    }
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




static  GMainLoop *loop;

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



int main(int argc, char const *argv[])
{
    openlog("rikmail", LOG_CONS, LOG_USER);

    // set up the SIGINT signal handler
    if (signal(SIGINT, &sig_handler) == SIG_ERR)
    {
        syslog(LOG_INFO, "Failed to register SIGINT handler, quitting...\n");
        exit(EXIT_FAILURE);
    }

    loop = g_main_loop_new (NULL, FALSE);
    guint bus_id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
                                  "xyz.openbmc_project.ares.rikmail",
                                  G_BUS_NAME_OWNER_FLAGS_NONE,
                                  on_bus_acquired,
                                  on_name_acquired,
                                  on_name_lost,
                                  NULL, NULL);

    // syslog(LOG_INFO, "Initial PID: %d\n", getpid());
    // syslog(LOG_INFO, "bus_id %u\n", bus_id);

    g_main_loop_run (loop);
    g_bus_unown_name(bus_id);

    g_main_loop_unref(loop);

    syslog(LOG_INFO, "Stop rikmail");

    return 0;
}
