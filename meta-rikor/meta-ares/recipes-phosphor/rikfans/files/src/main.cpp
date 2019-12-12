


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


std::mutex              g_lock;
std::condition_variable g_signal;
bool                    g_done;


class Zone
{
private:
	static const constexpr long long loop_min_delay = 300;

public:

	Zone() = delete;
	Zone(Zone &&that) = delete;
	void operator=(const Zone&) = delete;

	Zone(ec::pidinfo &pidinfo_initial,
	     std::vector<std::string> &s,
	     std::vector<std::string> &f,
	     long long ms) : sensors(s), pwms(f), setpt(20.0)
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

	ec::pid_info_t pid_info;
	double setpt;
	std::vector<std::string> sensors;
	std::vector<std::string> pwms;


	double processInputs()
	{

		std::ifstream ifs;
		int retval = 0;
		for (const auto &str : sensors)
		{
			int val = 100;
			ifs.open(str);
			if (ifs.is_open())
			{
				ifs >> val;
				if (!ifs.good())
				{
					val = 100;
				}
				ifs.close();
			}
			retval = std::max(retval, val);
		}
		return static_cast<double>(retval);
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
				ofs << val << "\n\n";
				dumpPIDStruct(ofs, &pid_info);
				ofs << std::endl;
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

			}

			auto input = zone->processInputs();
			auto output = zone->processPID(input);
			zone->processOutputs(output);

			std::this_thread::sleep_for(delay);
		}
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

	fs::path conf_fname = "/etc/rikfan/conf.json";
	if(!fs::exist(conf_fname)
	{
		conf_fname = "/tmp/rikfan/conf.json";
		if(!fs::exist(conf_fname))
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

	std::vector<std::unique_ptr<Zone>> zones;

	if (conf_json.count("zones") > 0)
	{
		for (const auto &z : conf_json["zones"])
		{
			ec::pidinfo pid_conf;
			std::vector<std::string> sens_vect;
			std::vector<std::string> pwm_vect;

			z["inputs"].get_to(sens_vect);
			z["fans_pwm"].get_to(pwm_vect);

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

			zones.emplace_back(std::make_unique<Zone>(pid_conf, sens_vect, pwm_vect, 3000));
		}
	}

	if(zones.size() == 0)
	{
		syslog(LOG_ERR, "No zone configurations found");
		std::cerr << "No zone configurations found";
		return -1;
	}

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
