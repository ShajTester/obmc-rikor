


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


std::mutex              g_lock;
std::condition_variable g_signal;
bool                    g_done;
bool                    cmd_quit;


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


class Zone
{
private:
	static const constexpr long long loop_min_delay = 300;
	static const constexpr double stop_output_const = 130.0;
	static const constexpr long margin_error_read_temp = 100000;

public:

	Zone() = delete;
	Zone(Zone &&that) = delete;
	void operator=(const Zone&) = delete;

	Zone(std::string n,
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

		if(type == "one")
			error_read_temp = 0;
		else
			error_read_temp = margin_error_read_temp;

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
#ifdef RIKFAN_DEBUG
		sample_time = std::chrono::system_clock::now();
#endif
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

	void command(const char *cmd)
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


private:
	bool manualmode;
	bool stop_flag;
	long long millisec;
	std::unique_ptr<std::thread> pmainthread;

	std::string name;
	std::string type;
	ec::pid_info_t pid_info;
	double setpt;
	std::vector<std::string> sensors;
	std::vector<std::string> pwms;
	long error_read_temp;

#ifdef RIKFAN_DEBUG
	decltype(std::chrono::system_clock::now()) sample_time;
#endif

	double processInputs()
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
		if(retval == 0)
			retval = margin_error_read_temp;

		return retval;
	}

	double processPID(double in)
	{
		return ec::pid(&pid_info, in, setpt);
	}

	void processOutputs(double in)
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

	static void zone_control_loop(Zone *zone)
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

};

static const constexpr auto myfifo = "/tmp/rikfan.pipe";

void *start_pipe(std::vector<std::unique_ptr<Zone>> *zones)
{
	int fd1;
	int rc;
	char str1[81];

	// Creating the named file(FIFO)
	// mkfifo(<pathname>,<permission>)
	while(mkfifo(myfifo, 0644))
	{
		syslog(LOG_ERR, "Can not create %s. Errno %d", myfifo, errno);
		unlink(myfifo);
		// fs::path pfifo { myfifo };
		if(fs::exists(myfifo))
			fs::remove(myfifo);
		sleep(5);
	}

	while (1)
	{
		// First open in read only and read
		fd1 = open(myfifo, O_RDONLY);
		rc = read(fd1, str1, 80);
		close(fd1);
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
#ifdef RIKFAN_DEBUG
			syslog(LOG_INFO, "Read string from pipe: <%s>", str1);
#endif
			if (std::strcmp(str1, "quit") == 0)
			{
				break;
			}
			else
			{
				std::for_each(zones->begin(), zones->end(), [str1](auto & zone) { zone->command(str1); });
			}
		}
	}

	unlink(myfifo);
	return nullptr;
}




void signalHandler( int signum )
{
	syslog(LOG_INFO, "Signal %d reached", signum);
	g_done = true;
	g_signal.notify_all();
}


int main(int argc, char const *argv[])
{
	std::vector<std::unique_ptr<Zone>> zones;

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

	// Для этого создаем файловый сокет '/tmp/rikfan.pipe'
	// В этот файл могут быть записаны следующие команды:
	//    * on
	//    * auto
	//    * manual

	auto cmd_thread = std::thread(&start_pipe, &zones);

	// Запускаем циклы управления вентиляторами по зонам
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



	auto fd1 = open(myfifo, O_WRONLY);
	write(fd1, "quit\0", 5);
	close(fd1);
	cmd_thread.join();


	syslog(LOG_INFO, "Stop zones control loop. (FAN)");

	return 0;
}
