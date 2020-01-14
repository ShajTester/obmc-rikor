

#include <iostream>
#include <fstream>
#include <iomanip>
#include <dirent.h>
#include <exception>
#include <string>
#include <random>
#include <chrono>
#include <ctime>

#include <set>

#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "json.hpp"
using json = nlohmann::json;
using namespace std::chrono_literals;

#include <rikor-fru.h>

#define EXPIRED_INTERVAL  180

// Отсюда
// https://stackoverflow.com/a/24586587
std::string random_string(std::string::size_type length)
{
    static auto& chrs = "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    // static auto& chrs = "123456789";
    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);
    std::string s;
    s.reserve(length);
    while(length--)
        s += chrs[pick(rg)];
    return s;
}



void print_blanc()
{
	std::cout << "{\"key\":\"\",\"lifetime\":\"\",\"login\":\"\"}" << std::endl;
	syslog(LOG_INFO, "{\"key\":\"\",\"lifetime\":\"\",\"login\":\"\"}");
}


void print_new_key(std::string &l)
{
	int fncnt = 0;
	auto jout = "{\"key\":\"\",\"lifetime\":\"\",\"login\":\"\"}"_json;
	std::string fname;
	std::set<std::string> fl;
	auto dir = opendir("/tmp/rikcgi-login/");
	if(dir != NULL)
	{
		auto entity = readdir(dir);
		while(entity != NULL)
		{
			if (entity->d_name[0] != '.') 
			{
				if(entity->d_type == DT_REG)
				{
					fl.emplace(entity->d_name);
				}
			}
			entity = readdir(dir);
		}
	}

	// for(const auto &it: fl) std::cout << it << "\n";

	fname = "/tmp/rikcgi-login/rcl0";
	if(fl.size() != 0)
	{
		auto ifn = fl.begin();
		while(ifn != fl.end())
		{
			fname = "rcl" + std::to_string(fncnt);
			ifn = fl.find(fname);
			fncnt++;
		}
		fname = "/tmp/rikcgi-login/" + fname;
	}

	auto newKey = random_string(8);
	auto expired = std::time(0) + EXPIRED_INTERVAL + 60;

	std::ofstream os;
	os.open(fname);
	if(!os)
	{
		mkdir("/tmp/rikcgi-login", S_IRWXU | S_IRWXG | S_IRWXO);
		os.open(fname);
	}
	if(!os)
	{
		syslog(LOG_ERR, "Can`t open tmp file %s", fname.c_str());
	}
	else
	{
		os << newKey << "\n";
		os << l << "\n";
		os << expired << "\n";
		os.close();
	}

	jout["key"] = newKey;
	jout["login"] = l;
	jout["lifetime"] = std::to_string(EXPIRED_INTERVAL * 1000);

	std::cout << jout;
	syslog(LOG_INFO, " ~~~ OUT %s", jout.dump().c_str());
}


bool psw_is_valid(std::string &l, std::string &p)
{
	int rf = 0;
	char fru_path[PATH_MAX];
	rikor_fru_t fru_data;
	rf = get_fru_device(fru_path);
	rf += read_fru(fru_path, &fru_data);
	if(rf != 0)
		fru_buf_init(&fru_data);

	if(l == "root")
	{
		return check_psw(rikor_fru_psw1, p.c_str(), &fru_data);
	}
	if(l == "admin")
	{
		return check_psw(rikor_fru_psw2, p.c_str(), &fru_data);
	}
	return false;
}


bool key_is_valid(std::string &l, const std::string &k, bool delete_key)
{
	bool retval = false;
	std::string fkey;
	std::string flogin;
	std::time_t expired;

	std::string fname;

	// Поиск файлов сессий
	auto dir = opendir("/tmp/rikcgi-login/");
	if(dir != NULL)
	{
		auto entity = readdir(dir);
		while(entity != NULL)
		{
			if (entity->d_name[0] != '.') 
			{
				if(entity->d_type == DT_REG)
				{
					fname = "/tmp/rikcgi-login/";
					fname += entity->d_name;
					std::ifstream f(fname);
					if(f.is_open())
					{
						f >> fkey;
						f >> flogin;
						f >> expired;
						f.close();
						auto tnow = std::time(0);
						if((fkey == k) && (expired > tnow))
						{
							retval = true;
							l = flogin;
							if(delete_key)
								if(remove(fname.c_str()) != 0)
									syslog(LOG_ERR, "Error deleting file %s", fname.c_str());
							// break;
						}
						else if(expired < tnow)
						{
							if(remove(fname.c_str()) != 0)
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


int main(int argc, char const *argv[])
{
    openlog("rikcgi-login", LOG_CONS, LOG_USER);
    // syslog(LOG_INFO, "rikcgi-login: started");
	

    std::string str;
    std::getline(std::cin, str);
	// syslog(LOG_INFO, " %s", str.c_str());
	json jin;
	try
	{
		jin = json::parse(str);
	}
	catch (std::exception& e)
	{
		syslog(LOG_ERR, "exception: %s", e.what());
	}

	syslog(LOG_INFO, " ~~~ IN %s", jin.dump().c_str());

	if(argc > 1)
	{
		if(std::strcmp(argv[1], "--setpassword") == 0)
		{
			syslog(LOG_INFO, " ~~~ set password");
			std::string in_cmd;
			std::string in_login;
			std::string in_key;
			std::string in_password1;
			std::string in_password2;
			json jout;

			if(jin.count("command") == 1)   in_cmd       = jin["command"].get<std::string>();
			if(jin.count("login") == 1)     in_login     = jin["login"].get<std::string>();
			if(jin.count("key") == 1)       in_key       = jin["key"].get<std::string>();
			if(jin.count("password1") == 1) in_password1 = jin["password1"].get<std::string>();
			// if(jin.count("password2") == 1) in_password2 = jin["password"].get<std::string>();

			if(key_is_valid(in_login, in_key, false) && (in_cmd == "setpassword"))
			{
				int rf = 0;
				char fru_path[PATH_MAX];
				rikor_fru_t fru_data;
				rf = get_fru_device(fru_path);
				rf += read_fru(fru_path, &fru_data);
				if(rf != 0)
					fru_buf_init(&fru_data);

				rf = set_psw(rikor_fru_psw1, "", in_password1.c_str(), &fru_data);
				rf += write_fru(fru_path, &fru_data);
				if(rf == 0)
				{
					jout["password1"] = "Password change successful";
					jout["key"] = "";
					jout["login"] = "";
					syslog(LOG_INFO, "New password: Password change successful");
				}
				else
				{
					jout["password1"] = "EEPROM write error";
					// jout["key"] = "";
					// jout["login"] = "";
					syslog(LOG_ERR, "New password: EEPROM write error");
				}
			}
			else
			{
				jout["password1"] = "KEY error";
				// jout["key"] = "";
				// jout["login"] = "";
				syslog(LOG_ERR, "New password: KEY error");
			}

			std::cout << jout;
		}
		else
		{
			syslog(LOG_ERR, "rikcgi-login argument error");
		}
	}
	else
	{
		std::string in_cmd;
		std::string in_key;
		std::string in_login;
		std::string in_psw;

		if(jin.count("command") == 1) in_cmd = jin["command"].get<std::string>();
		if(jin.count("login") == 1) in_login = jin["login"].get<std::string>();
		if(jin.count("key") == 1) in_key = jin["key"].get<std::string>();
		if(jin.count("password") == 1) in_psw = jin["password"].get<std::string>();

		if(in_cmd.length() == 0)
		{
			print_blanc();
		}
		else if(in_cmd == "relogin")
		{
			print_blanc();
		}
		else if(in_cmd == "login")
		{
			if(psw_is_valid(in_login, in_psw))
				print_new_key(in_login);
			else
				print_blanc();
		}
		else if(in_cmd == "logout")
		{
			key_is_valid(in_login, in_key, true);
			print_blanc();
		}
		else if(in_cmd == "update")
		{
			if(key_is_valid(in_login, in_key, true))
				print_new_key(in_login);
			else
				print_blanc();
		}
		else
		{
			print_blanc();
		}
	}

	return 0;
}


