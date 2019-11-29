


#include <iostream>
#include <fstream>
#include <filesystem>
#include <exception>
#include <string>
#include <set>

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
	for (int i = 1; i < 9; i++)
	{
		pFT = json::array();
		std::sprintf(fname, "Fan %d", i);
		pFT += fname;

		readVal = 0;
		std::sprintf(fname, FanFileFmt, i);
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
		std::sprintf(fname, PWMFileFmt, i);
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
					json newPwm;
					// Изменение задания в ручном режиме
					for (int i = 1; i < 9; i++)
					{
						// Чтение текущих значений PWM
						std::sprintf(cstr, PWMFileFmt, i);
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

						std::sprintf(cstr, "fan%d", i);
						if (jin.count(cstr) > 0)
						{
							readVal = rawPWM(jin[cstr].get<std::string>());
						}

						if (readVal < minPWMraw)
							readVal = minPWMraw;

						std::sprintf(cstr, PWMFileFmt, i);
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

						newPwm["pwm" + std::to_string(i)] = readVal;
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

			}
		}

		fillState(jout);
		std::cout << jout << std::endl;

		syslog(LOG_INFO, " ~~~ OUT %s", jout.dump().c_str());
	}
	return 0;
}


