


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

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <nlohmann/json.hpp>

#include "ec/pid.hpp"
#include "util.hpp"

namespace fs = std::filesystem;
using namespace std::literals::chrono_literals;
using json = nlohmann::json;

#define RIKFAN_DEBUG


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

// Отсюда
// https://stackoverflow.com/a/21995693
template<typename TimeT = std::chrono::milliseconds>
struct measure
{
	template<typename F, typename ...Args>
	static typename TimeT::rep execution(F&& func, Args&&... args)
	{
		auto start = std::chrono::steady_clock::now();
		std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
		auto duration = std::chrono::duration_cast< TimeT>
							(std::chrono::steady_clock::now() - start);
		return duration.count();
	}
};



std::string resultpath;

int main(int argc, char const *argv[])
{

	std::vector<std::string> paths {
		"/tmp/rikfan/s1/*/f1",
		"/tmp/rikfan/s2/*/f1",
		"/tmp/rikfan/s3/*/f1",
		"/tmp/rikfan/s4/*/f1",
		"/tmp/rikfan/s5/*/f1"
	};

	for(const auto &str: paths)
	{
		get_sensor_path(resultpath, str);
		std::cout << resultpath << std::endl;
	}

	std::vector<std::string> p2 {
        "/sys/bus/i2c/devices/6-0049/*/temp1_input",
        "/sys/bus/i2c/devices/6-004b/*/temp1_input",
        "/sys/bus/i2c/devices/6-004c/*/temp1_input",
        "/sys/bus/i2c/devices/6-004e/*/temp1_input",
        "/sys/bus/i2c/devices/6-004f/*/temp1_input",
        "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/*/pwm1",
        "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/*/pwm2",
        "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/*/pwm3",
        "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/*/pwm4",
        "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/*/pwm5",
        "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/*/pwm6",
        "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/*/pwm7",
        "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/*/pwm8"
	};


	for(const auto &str: p2)
	{
		get_sensor_path(resultpath, str);
		std::cout << resultpath << std::endl;
	}

	for(int i=0; i<10; i++)
	{
		std::cout << "100 * " << p2.size() << " converted " << measure<std::chrono::microseconds>::execution([&]()
			{
				for(int j=0; j<100; j++)
				{
					for(const auto &str: p2)
					{
						get_sensor_path(resultpath, str);
					}
				}
			}) << " us\n";
	}


	return 0;
}