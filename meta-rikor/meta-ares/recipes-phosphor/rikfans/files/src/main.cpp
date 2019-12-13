


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
#include <nlohmann/json.hpp>

#include "ec/pid.hpp"
#include "util.hpp"

namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;
using json = nlohmann::json;

#define RIKFAN_DEBUG


std::mutex              g_lock;
std::condition_variable g_signal;
bool                    g_done;


class Zone
{
private:
	static const constexpr long long loop_min_delay = 300;
	static const constexpr double stop_output_const = 130.0;
	static const constexpr long error_read_temp = 100000;

public:

	Zone() = delete;
	Zone(Zone &&that) = delete;
	void operator=(const Zone&) = delete;

	Zone(std::string n,
	     ec::pidinfo &pidinfo_initial,
	     std::vector<std::string> &s,
	     std::vector<std::string> &f,
	     double sp,
	     long long ms
	    ) : name(n), sensors(s), pwms(f), setpt(sp)
	{
		pmainthread = nullptr;
		if (ms < loop_min_delay)
			millisec = loop_min_delay;
		else
			millisec = ms;

		initializePIDStruct(&pid_info, pidinfo_initial);
	}

	~Zone()
	{
		stop();
	}

	void start()
	{
		if (pmainthread != nullptr)
			stop();
		stop_flag = false;
		manualmode = false;
		initialized = false;
		pmainthread = std::make_unique<std::thread>(&Zone::zone_control_loop, this);
	}

	void stop()
	{
		if (pmainthread != nullptr)
		{
			stop_flag = true;
			pmainthread->join();
			pmainthread = nullptr;
		}
	}


private:
	bool manualmode;
	bool initialized;
	bool stop_flag;
	long long millisec;
	std::unique_ptr<std::thread> pmainthread;

	std::string name;
	ec::pid_info_t pid_info;
	double setpt;
	std::vector<std::string> sensors;
	std::vector<std::string> pwms;


	double processInputs()
	{

		std::ifstream ifs;
		double retval = 0;
		for (const auto &str : sensors)
		{
			long val;
			ifs.open(str);
			if (ifs.is_open())
			{
				ifs >> val;
				if (!ifs.good())
				{
					val = error_read_temp;
				}
				ifs.close();
			}
			else
			{
				val = error_read_temp;
			}
			retval = std::max(retval, (static_cast<double>(val) / 1000.0));
		}
		return retval;
	}

	double processPID(double in)
	{
		return ec::pid(&pid_info, in, setpt);
	}

	void processOutputs(double in)
	{
		std::ofstream ofs;
		int val = static_cast<int>(std::round(in));
		for (const auto &str : pwms)
		{
			ofs.open(str);
			if (ofs.is_open())
			{
				ofs << val;
				ofs.close();
			}
		}
	}

	static void zone_control_loop(Zone *zone)
	{
		auto delay = std::chrono::milliseconds(zone->millisec);
		while (!zone->stop_flag)
		{
			// if (zone->manualmode)
			// {
			// 	zone->initialized = false;
			// 	std::this_thread::sleep_for(delay);
			// 	continue;
			// }

			if (!zone->initialized)
			{
				// For future use
				zone->initialized = true;
			}

			auto input = zone->processInputs();
			auto output = zone->processPID(input);
			zone->processOutputs(output);

#ifdef RIKFAN_DEBUG
			std::ofstream ofs;
			ofs.open(fs::path("/tmp/rikfan") / zone->name);
			if (ofs.is_open())
			{
				ofs << "setpoint: " << zone->setpt;
				ofs << "\ninput:    " << input;
				ofs << "\noutput:   " << output;
				ofs << "\n\n";
				dumpPIDStruct(ofs, &zone->pid_info);
				ofs << std::endl;
				ofs.close();
			}
#endif // RIKFAN_DEBUG		

			std::this_thread::sleep_for(delay);
		}
		zone->processOutputs(stop_output_const);
	}

};

void signalHandler( int signum )
{
	syslog(LOG_INFO, "Signal %d reached", signum);
	g_done = true;
	g_signal.notify_all();
}


int main(int argc, char const *argv[])
{
	openlog("rikfan", LOG_CONS, LOG_USER);



	std::vector<std::unique_ptr<Zone>> zones;

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

				z["inputs"].get_to(sens_vect);
				z["fans_pwm"].get_to(pwm_vect);
				z["name"].get_to(zone_name);
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

				zones.emplace_back(std::make_unique<Zone>(zone_name, pid_conf, sens_vect, pwm_vect, setpoint, pid_conf.ts * 1000));
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
		if(!fs::exists(debug_path))
		{
			fs::create_directory(debug_path);
		}
	}
#endif // RIKFAN_DEBUG


	// TODO:
	// Основной цикл должен обрабатывать внешние события.
	// Такие как:
	//    * включение - запуск управления вентиляторами;
	//    * отключение
	//    * переход в ручное управление.

	for (auto &z : zones)
		z->start();

	syslog(LOG_INFO, "Started control loop for %ld zones", zones.size());

	g_done = false;

	std::signal(SIGINT, signalHandler);
	std::signal(SIGABRT, signalHandler);
	std::signal(SIGCONT, signalHandler);
	std::signal(SIGSTOP, signalHandler);
	std::signal(SIGKILL, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	std::signal(SIGTERM, signalHandler);

	std::unique_lock<std::mutex> lock(g_lock);
	while (!g_done)
		g_signal.wait(lock);

	syslog(LOG_INFO, "Stop zones control loop. (FAN)");

	// std::string instr;
	// std::cin >> instr;
	// std::cout << "\n\n\tProgramm complete!\n";

	return 0;
}
