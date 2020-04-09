
#pragma once

#include "ec/pid.hpp"
#include "util.hpp"



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
	    );

	~Zone();
	void start();
	void stop();
	void command(const char *cmd);


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

	double processInputs();
	double processPID(double in);
	void processOutputs(double in);
	static void zone_control_loop(Zone *zone);
};

