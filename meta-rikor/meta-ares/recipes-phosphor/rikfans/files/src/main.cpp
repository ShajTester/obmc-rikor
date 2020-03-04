


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

#include "zone.hpp"
#include "fan.h"

/// gDBus
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include "rikfan-manager.h"


namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;
using json = nlohmann::json;

// #define RIKFAN_DEBUG

static 	std::vector<std::unique_ptr<Zone>> zones;


void setFanmode(unsigned int mode)
{
	if (mode > 3)
		mode = 0;   // 0 - автоматический режим

	if (mode == 0)
	{
		for (const auto &z : zones)
			z->command("auto");
	}
	else
	{
		for (const auto &z : zones)
			z->command("manual");
		// Вручную установить значения PWM
		setPWM(mode);
	}
}




static gboolean on_handle_apply_mode (XyzOpenbmc_projectAresRikfan *interface, GDBusMethodInvocation *invocation,
                                      const gchar *greeting, gpointer user_data)
{
	gchar *response;
	unsigned int mode = 0;
	auto cur_mode = xyz_openbmc_project_ares_rikfan_get_fan_mode(interface);
	// syslog(LOG_INFO, "Was %s   -   new %s", cur_mode, greeting);

	try
	{
		mode = std::stoi(greeting);
	}
	catch (const std::exception &e)
	{
		mode = 0;
	}

	setFanmode(mode);
	response = g_strdup(greeting);
	xyz_openbmc_project_ares_rikfan_set_fan_mode(interface, response);
	xyz_openbmc_project_ares_rikfan_complete_apply_mode (interface, invocation, response);
	g_free (response);

	return TRUE;
}


static gboolean on_handle_reset_pid (XyzOpenbmc_projectAresRikfan *interface, GDBusMethodInvocation *invocation,
                                     const gchar *greeting, gpointer user_data)
{
	for (const auto &z : zones)
		z->command("on");
	xyz_openbmc_project_ares_rikfan_complete_reset_pid (interface, invocation);
	return TRUE;
}




static void on_bus_acquired (GDBusConnection *connection,
                             const gchar     *name,
                             gpointer         user_data)
{
	XyzOpenbmc_projectAresRikfan *interface;
	GError *error;

	/* This is where we'd export some objects on the bus */
	syslog(LOG_INFO, "on_bus_acquired %s on the session bus\n", name);


	gchar *conn_name;
	g_object_get(connection, "unique-name", &conn_name, NULL);
	syslog(LOG_INFO, "%s\n", conn_name);
	g_free(conn_name);

	interface = xyz_openbmc_project_ares_rikfan_skeleton_new();
	g_signal_connect (interface, "handle-apply-mode", G_CALLBACK (on_handle_apply_mode), NULL);
	g_signal_connect (interface, "handle-reset-pid", G_CALLBACK (on_handle_reset_pid), NULL);
	error = NULL;
	if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (interface), connection, "/xyz/openbmc_project/ares/rikfan", &error))
	{
		g_print("ERROR %s\n", error->message);
	}
	// gchar * fanmode;
	xyz_openbmc_project_ares_rikfan_set_fan_mode(interface, "0");
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



int main(int argc, char const *argv[])
{
	openlog("rikfan", LOG_CONS, LOG_USER);

	{
		fs::path conf_fname = "/etc/rikfan/conf.json";
		if (!fs::exists(conf_fname))
		{
			conf_fname = "/tmp/rikfan/conf.json";
			if (!fs::exists(conf_fname))
			{
				syslog(LOG_ERR, "Need config file in '/etc/rikfan/conf.json' or '/tmp/rikfan/conf.json'");
				std::cerr << "Need config file in '/etc/rikfan/conf.json' or '/tmp/rikfan/conf.json'";
				return -1;
			}
		}

		std::ifstream conf_stream {conf_fname};
		json conf_json;
		try
		{
			conf_stream >> conf_json;
		}
		catch (const std::exception &e)
		{
			// std::cerr << e.what() << std::endl;
			syslog(LOG_ERR, "exception: %s", e.what());
		}

		if (conf_json.count("zones") > 0)
		{
			for (const auto &z : conf_json["zones"])
			{
				ec::pidinfo pid_conf;
				std::vector<std::string> sens_vect;
				std::vector<std::string> pwm_vect;
				double setpoint;
				std::string zone_name;
				std::string zone_type;

				z["inputs"].get_to(sens_vect);
				z["fans_pwm"].get_to(pwm_vect);
				z["name"].get_to(zone_name);
				z["type"].get_to(zone_type);
				z["setpoint"].get_to(setpoint);

				auto p = z["pid"];
				p["samplePeriod"].get_to(pid_conf.ts);
				p["proportionalCoeff"].get_to(pid_conf.proportionalCoeff);
				p["integralCoeff"].get_to(pid_conf.integralCoeff);
				p["feedFwdOffsetCoeff"].get_to(pid_conf.feedFwdOffset);
				p["feedFwdGainCoeff"].get_to(pid_conf.feedFwdGain);
				p["integralLimit_min"].get_to(pid_conf.integralLimit.min);
				p["integralLimit_max"].get_to(pid_conf.integralLimit.max);
				p["outLim_min"].get_to(pid_conf.outLim.min);
				p["outLim_max"].get_to(pid_conf.outLim.max);
				p["slewNeg"].get_to(pid_conf.slewNeg);
				p["slewPos"].get_to(pid_conf.slewPos);

				zones.emplace_back(std::make_unique<Zone>(zone_name, zone_type, pid_conf, sens_vect, pwm_vect, setpoint, pid_conf.ts * 1000));
			}
		}
	}


	if (zones.size() == 0)
	{
		syslog(LOG_ERR, "No zone configurations found");
		std::cerr << "No zone configurations found";
		return -1;
	}

#ifdef RIKFAN_DEBUG
	{
		fs::path debug_path("/tmp/rikfan");
		if (!fs::exists(debug_path))
		{
			fs::create_directory(debug_path);
		}
	}
#endif // RIKFAN_DEBUG

	// Запускаем циклы управления вентиляторами по зонам
	for (auto &z : zones)
		z->start();

	syslog(LOG_INFO, "Started control loop for %ld zones", zones.size());

	// set up the SIGINT signal handler
	if (signal(SIGINT, &sig_handler) == SIG_ERR)
	{
		syslog(LOG_INFO, "Failed to register SIGINT handler, quitting...\n");
		exit(EXIT_FAILURE);
	}

	loop = g_main_loop_new (NULL, FALSE);

	// guint bus_id = g_bus_own_name(G_BUS_TYPE_SESSION, "com.rikor", G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
	//                on_name_acquired, on_name_lost, NULL, NULL);
	guint bus_id = g_bus_own_name(G_BUS_TYPE_SYSTEM, "xyz.openbmc_project.ares.rikfan", G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
	                              on_name_acquired, on_name_lost, NULL, NULL);

	// syslog(LOG_INFO, "Initial PID: %d\n", getpid());
	// syslog(LOG_INFO, "bus_id %u\n", bus_id);

	g_main_loop_run (loop);
	g_bus_unown_name(bus_id);

	g_main_loop_unref(loop);

	syslog(LOG_INFO, "Stop zones control loop. (FAN)");

	return 0;
}
