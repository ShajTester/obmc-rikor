


#include <iostream>
#include <fstream>
#include <filesystem>
#include <exception>
#include <string>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "json.hpp"

namespace fs = std::filesystem;

using json = nlohmann::json;
using namespace std::chrono_literals;


bool key_is_valid(std::string &l, const std::string &k, bool delete_key)
{
	bool retval = false;
	std::string fkey;
	std::string flogin;
	std::time_t expired;

	std::string fname;

	// Поиск файлов сессий
	auto dir = opendir("/tmp/rikcgi-login/");
	if (dir != NULL)
	{
		auto entity = readdir(dir);
		while (entity != NULL)
		{
			if (entity->d_name[0] != '.')
			{
				if (entity->d_type == DT_REG)
				{
					fname = "/tmp/rikcgi-login/";
					fname += entity->d_name;
					std::ifstream f(fname);
					if (f.is_open())
					{
						f >> fkey;
						f >> flogin;
						f >> expired;
						f.close();
						auto tnow = std::time(0);
						if ((fkey == k) && (expired > tnow))
						{
							retval = true;
							l = flogin;
							if (delete_key)
								if (remove(fname.c_str()) != 0)
									syslog(LOG_ERR, "Error deleting file %s", fname.c_str());
							// break;
						}
						else if (expired < tnow)
						{
							if (remove(fname.c_str()) != 0)
								syslog(LOG_ERR, "Error deleting file %s", fname.c_str());
						}
					}
				}
			}
			entity = readdir(dir);
		}
	}
	return retval;
}


#define CONFIG_FILE_NAME "/usr/local/fan.conf"
#define minPWMraw   (40)
#define nomPWMraw   (120)
#define PWMFileFmt  "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/pwm%d"
#define FanFileFmt  "/sys/bus/platform/devices/1e786000.pwm-tacho-controller/hwmon/hwmon0/fan%d_input"


struct FANDescr
{
	std::string name;
	std::string webname;
	int tach;
	int pwm;
};

const std::vector<FANDescr> fanDescr {
	{"Fan 1", "fan1", 1, 1},
	{"Fan 2", "fan2", 2, 2},
	{"Fan 3", "fan3", 3, 3},
	{"Fan 4", "fan4", 4, 4},
	{"Fan 5", "fan5", 8, 5},
	{"Fan 6", "fan6", 7, 6},
	{"CPU 0", "fan7", 6, 7},
	{"CPU 1", "fan8", 5, 8}
};

int rawPWM(std::string perc)
{
	int readVal;
	try
	{
		readVal = std::lround(std::stoi(perc) * 255.0 / 100.0);
	}
	catch (const std::exception &e)
	{
		readVal = nomPWMraw;
	}
	return readVal;
}

std::string percPWM(int raw)
{
	if (raw < 0)
		return std::to_string(raw);
	return std::to_string(std::lround(raw * 100.0 / 255.0));
}


std::string percFAN(int raw)
{
	return std::to_string(raw);
}


int fillState(json &jdata)
{
	std::ifstream ifd;
	int readVal;
	json pFT;
	char fname[256];

	jdata["fantach"] = json::array();
	for (const auto &it: fanDescr)
	{
		pFT = json::array();
		pFT += it.name;

		readVal = 0;
		std::sprintf(fname, FanFileFmt, it.tach);
		ifd.open(fname);
		if (ifd.is_open())
		{
			ifd >> readVal;
			if (!ifd.good())
				readVal = -1;
			ifd.close();
		}
		else
		{
			readVal = -2;
		}
		pFT += percFAN(readVal);

		readVal = 0;
		std::sprintf(fname, PWMFileFmt, it.pwm);
		ifd.open(fname);
		if (ifd.is_open())
		{
			ifd >> readVal;
			if (!ifd.good())
				readVal = -1;
			ifd.close();
		}
		pFT += percPWM(readVal);

		jdata["fantach"].emplace_back(pFT);
	}
	return 0;
}




int main(int argc, char const *argv[])
{
	openlog("rikcgi-fan", LOG_CONS, LOG_USER);


	if (argc > 1)
	{
		if (std::strcmp(argv[1], "--getall") == 0)
		{
			//
			json jout;
			jout["fanauto"] = "on";
			fillState(jout);
			std::cout << jout["fantach"] << std::endl;
		}
		else if (std::strcmp(argv[1], "--init") == 0)
		{
			json jconf;

			std::ifstream confin {CONFIG_FILE_NAME};
			try
			{
				confin >> jconf;
				confin.close();
			}
			catch (std::exception& e)
			{
				if (confin.is_open())
					confin.close();
				jconf.clear();
				jconf["pwm1"] = nomPWMraw;
				jconf["pwm2"] = nomPWMraw;
				jconf["pwm3"] = nomPWMraw;
				jconf["pwm4"] = nomPWMraw;
				jconf["pwm5"] = nomPWMraw;
				jconf["pwm6"] = nomPWMraw;
				jconf["pwm7"] = nomPWMraw;
				jconf["pwm8"] = nomPWMraw;
				std::ofstream confout {CONFIG_FILE_NAME};
				confout << jconf;
				confout.close();
			}

			auto pwmPath = fs::path(PWMFileFmt).remove_filename();
			std::ofstream pwmFile;
			for (const auto &it : jconf.items())
			{
				// syslog(LOG_INFO, "Key <%s>   value <%d>", it.key().c_str(), it.value().get<int>());
				pwmFile.open(pwmPath / it.key());
				if (pwmFile.is_open())
				{
					pwmFile << it.value().get<int>();
					pwmFile.close();
				}
			}
		}
	}
	else
	{
		std::string str;
		json jin;
		json jout;
		char cstr[256];
		std::fstream fd;
		int readVal;

		std::getline(std::cin, str);
		try
		{
			jin = json::parse(str);
		}
		catch (std::exception& e)
		{
			syslog(LOG_ERR, "exception: %s", e.what());
		}

		syslog(LOG_INFO, " ~~~ IN %s", jin.dump().c_str());

		if (jin.count("fanauto") > 0)
		{
			jout["fanauto"] = jin["fanauto"];

			if (jin["fanauto"] == std::string("off"))
			{
				// Включен ручной режим управления
				// Для изменения задания нужно проверить ключ
				std::string in_login;
				std::string in_key;
				if (jin.count("key") == 1) in_key = jin["key"].get<std::string>();
				jout["key"] = in_key;

				if (key_is_valid(in_login, in_key, false))
				{
					// Отключение автоматического режима
					fd.open("/tmp/rikfan.pipe", std::ios::out);
					fd << "manual";
					fd.close();


					int num;
					json newPwm;
					// Изменение задания в ручном режиме
					for(const auto &it: fanDescr)
					{
						// Чтение текущих значений PWM
						std::sprintf(cstr, PWMFileFmt, it.pwm);
						fd.open(cstr, std::ios::in);
						if (fd.is_open())
						{
							fd >> readVal;
							if (!fd.good())
								readVal = nomPWMraw;
							fd.close();
						}
						else
						{
							readVal = nomPWMraw;
						}

						if (jin.count(it.webname) > 0)
						{
							readVal = rawPWM(jin[it.webname].get<std::string>());
						}

						if (readVal < minPWMraw)
							readVal = minPWMraw;

						std::sprintf(cstr, PWMFileFmt, it.pwm);
						fd.open(cstr, std::ios::out);

						if (fd.is_open())
						{
							fd << readVal;
							if (!fd.good())
							{
								syslog(LOG_ERR, "Error %d write to file <%s>", errno, cstr);
							}
							fd.close();
						}
						else
						{
							syslog(LOG_ERR, "Error %d open file <%s>", errno, cstr);
						}

						newPwm["pwm" + std::to_string(it.pwm)] = readVal;
					} // for

					std::ofstream confout {CONFIG_FILE_NAME};
					confout << newPwm;
					confout.close();

				} // is key valid
				else
				{
					syslog(LOG_ERR, "Key <%s> is not valid", in_key);
				}
			} // fanauto = off
			else
			{
				// fanauto = on
				// Включение автоматического режима
				fd.open("/tmp/rikfan.pipe", std::ios::out);
				fd << "auto";
				fd.close();

			}
		}

		fillState(jout);
		std::cout << jout << std::endl;

		syslog(LOG_INFO, " ~~~ OUT %s", jout.dump().c_str());
	}
	return 0;
}


