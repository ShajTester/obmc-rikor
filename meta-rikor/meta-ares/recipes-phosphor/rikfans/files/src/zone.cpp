


#include <iostream>
#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cmath>

#include <vector>
#include <functional>
#include <filesystem>

#include <nlohmann/json.hpp>

#include "ec/pid.hpp"
#include "util.hpp"

#include "zone.hpp"


namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;
using json = nlohmann::json;

// #define RIKFAN_DEBUG


void get_sensor_path(std::string &result, const std::string &str)
{
	auto pos = str.find('*');
	if (pos == std::string::npos)
	{
		result = str;
		return;
	}

	auto ppos = str.rfind('/', pos);
	auto fpos = str.find('/', pos);
	try
	{
		for (const auto &p : fs::directory_iterator(str.substr(0, ppos) + "/hwmon/"))
		{
			result = (p.path() / str.substr(++fpos)).string();
			break;
		}
	}
	catch (std::exception &e)
	{
		result = str;
	}
}



Zone::Zone(std::string n,
           std::string t,
           ec::pidinfo &pidinfo_initial,
           std::vector<std::string> &s,
           std::vector<std::string> &f,
           double sp,
           long long ms
          ) : sensors(s), pwms(f), name(n), type(t), setpt(sp)
{
	pmainthread = nullptr;
	if (ms < loop_min_delay)
		millisec = loop_min_delay;
	else
		millisec = ms;

	if (type == "one")
		error_read_temp = 0;
	else
		error_read_temp = margin_error_read_temp;

	initializePIDStruct(&pid_info, pidinfo_initial);
}

Zone::~Zone()
{
	stop();
}

void Zone::start()
{
	if (pmainthread != nullptr)
		stop();
	stop_flag = false;
	manualmode = false;
#ifdef RIKFAN_DEBUG
	sample_time = std::chrono::system_clock::now();
#endif
	pmainthread = std::make_unique<std::thread>(&Zone::zone_control_loop, this);
}

void Zone::stop()
{
	if (pmainthread != nullptr)
	{
		stop_flag = true;
		pmainthread->join();
		pmainthread = nullptr;
	}
}

void Zone::command(const char *cmd)
{
	if (std::strcmp(cmd, "manual") == 0)
	{
		manualmode = true;
	}
	else if (std::strcmp(cmd, "auto") == 0)
	{
		manualmode = false;
	}
	else if (std::strcmp(cmd, "on") == 0)
	{
		pid_info.lastOutput = pid_info.outLim.min;
	}
}



double Zone::processInputs()
{
	std::string actual_path;
	std::ifstream ifs;
	double retval = 0;
	for (const auto &str : sensors)
	{
		long val;
		get_sensor_path(actual_path, str);
		ifs.open(actual_path);
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

	// for type == "one"
	if (retval == 0)
		retval = margin_error_read_temp;

	return retval;
}

double Zone::processPID(double in)
{
	return ec::pid(&pid_info, in, setpt);
}

void Zone::processOutputs(double in)
{
	std::string actual_path;
	std::ofstream ofs;
	int val = static_cast<int>(std::round(in));
	for (const auto &str : pwms)
	{
		get_sensor_path(actual_path, str);
		ofs.open(actual_path);
		if (ofs.is_open())
		{
			ofs << val;
			ofs.close();
		}
	}
}

void Zone::zone_control_loop(Zone *zone)
{
	auto delay = std::chrono::milliseconds(zone->millisec);
	while (!zone->stop_flag)
	{
		if (zone->manualmode)
		{
			std::this_thread::sleep_for(delay);
		}
		else
		{
			auto input = zone->processInputs();
			auto output = zone->processPID(input);
			zone->processOutputs(output);

#ifdef RIKFAN_DEBUG

			auto new_sample = std::chrono::system_clock::now();

			std::ofstream ofs;
			ofs.open(fs::path("/tmp/rikfan") / zone->name);
			if (ofs.is_open())
			{
				ofs << "setpoint: " << zone->setpt;
				ofs << "\ninput:    " << input;
				ofs << "\noutput:   " << output;
				std::chrono::duration<double> diff = new_sample - zone->sample_time;
				zone->sample_time = new_sample;
				ofs << "\nsample_time: " << diff.count();
				ofs << "\n\n";
				dumpPIDStruct(ofs, &zone->pid_info);
				ofs << std::endl;
				ofs.close();
			}
#endif // RIKFAN_DEBUG		

			std::this_thread::sleep_for(delay);
		}
	}
	zone->processOutputs(stop_output_const);
}


