

// #define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <dirent.h>
#include <exception>
#include <string>
#include <cstring>
#include <random>
#include <chrono>
#include <ctime>

#include <set>

#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>

#include "json.hpp"

#include <rikor-fru.h>

using json = nlohmann::json;
using namespace std::chrono_literals;



int GetDefaultGw ( std::string & gw )
{
    FILE *f;
    char line[100] , *p , *c, *g, *saveptr;
    int nRet=1;

    f = fopen("/proc/net/route" , "r");

    while(fgets(line , 100 , f))
    {
        p = strtok_r(line , " \t", &saveptr);
        c = strtok_r(NULL , " \t", &saveptr);
        g = strtok_r(NULL , " \t", &saveptr);

        if(p!=NULL && c!=NULL)
        {
            if(strcmp(c , "00000000") == 0)
            {
                //printf("Default interface is : %s \n" , p);
                if (g)
                {
                    char *pEnd;
                    int ng=strtol(g,&pEnd,16);
                    //ng=ntohl(ng);
                    struct in_addr addr;
                    addr.s_addr=ng;
                    gw=std::string( inet_ntoa(addr) );
                    nRet=0;
                }
                break;
            }
        }
    }

    fclose(f);
    return nRet;
}


int fill_net_info(json &jout)
{
	struct ifaddrs *ifaddr, *ifa;
	int family, s, n;
	char host[NI_MAXHOST];

	int rf = 0;
	char fru_path[PATH_MAX];
	rikor_fru_t fru_data;
	rf = get_fru_device(fru_path);
	rf += read_fru(fru_path, &fru_data);
	if(rf != 0)
		fru_buf_init(&fru_data);
	if(fru_data.dhcp1 == 1)
		jout["dhcp"] = "yes";
	else
		jout["dhcp"] = "no";

	gethostname(host, NI_MAXHOST);
	jout["hostname"] = host;

	std::string gwstr;
	if(GetDefaultGw(gwstr) == 0)
	{
		jout["gateway"] = gwstr;
		jout["dns"] = gwstr;
	}

	if (getifaddrs(&ifaddr) == -1) 
	{
		syslog(LOG_ERR, "getifaddrs error");
		return -1;
	}

	/* Walk through linked list, maintaining head pointer so we can free list later */
	for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) 
	{
		if (ifa->ifa_addr == NULL)
			continue;

		if(strcmp(ifa->ifa_name, "eth0") == 0)
		{
			family = ifa->ifa_addr->sa_family;
			if(family == AF_INET)
			{
				s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if(s != 0)
					syslog(LOG_ERR, "getnameinfo() failed: %s", gai_strerror(s));
				else
					jout["addr"] = host;

				s = getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if(s != 0)
					syslog(LOG_ERR, "getnameinfo() failed: %s", gai_strerror(s));
				else
					jout["mask"] = host;
			}
			else if(family == AF_INET6)
			{
				s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if(s != 0)
					syslog(LOG_ERR, "getnameinfo() failed: %s", gai_strerror(s));
				else
					jout["ip6_addr"] = host;

				// s = getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				// if(s != 0)
				// 	std::cout << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
				// else
				// 	std::cout << ifa->ifa_name << "\tnetmask: <" << host << ">" << std::endl;
			}
			else if (family == AF_PACKET)
			{
				struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
				char *pstr = host;
				for (int i=0; i < s->sll_halen; i++)
				{
					pstr += sprintf(pstr, "%02x%c", (s->sll_addr[i]), (i+1!=s->sll_halen)?':':'\0');
				}
				jout["mac"] = host;
	        }
	    }
	}
	freeifaddrs(ifaddr);

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
    openlog("rikcgi-net", LOG_CONS, LOG_USER);


	if(argc > 1)
	{
		if(std::strcmp(argv[1], "--store") == 0)
		{
		    std::string str;
		    std::getline(std::cin, str);
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

			std::string in_login;
			std::string in_key;

			if(jin.count("login") == 1)     in_login     = jin["login"].get<std::string>();
			if(jin.count("key") == 1)       in_key       = jin["key"].get<std::string>();

			if(key_is_valid(in_login, in_key, false))
			{
				int rf = 0;
				char fru_path[PATH_MAX];
				rikor_fru_t fru_data;
				rf = get_fru_device(fru_path);
				rf += read_fru(fru_path, &fru_data);
				if(rf != 0)
				{
					syslog(LOG_ERR, "FRU read error");
					fru_buf_init(&fru_data);
				}

				if(jin.count("hostname") > 0)
				{
					strcpy(fru_data.hostname, jin["hostname"].get<std::string>().c_str());
				}
				if(jin.count("addr") > 0)
				{
					inet_aton(jin["addr"].get<std::string>().c_str(), &fru_data.ip1);
				}
				if(jin.count("mask") > 0)
				{
					inet_aton(jin["mask"].get<std::string>().c_str(), &fru_data.netmask1);
				}
				if(jin.count("gateway") > 0)
				{
					inet_aton(jin["gateway"].get<std::string>().c_str(), &fru_data.gate1);
				}
				if(jin.count("dns") > 0)
				{
					inet_aton(jin["dns"].get<std::string>().c_str(), &fru_data.dns1_1);
				}
				if(jin.count("dhcp") > 0)
				{
					if(jin["dhcp"] == "yes")
						fru_data.dhcp1 = 1;
					else
						fru_data.dhcp1 = 0;
				}
				rf = write_fru(fru_path, &fru_data);
				if(rf != 0)
					syslog(LOG_ERR, "FRU write error");
			}
			else
			{
				syslog(LOG_ERR, "Invaid key");
			}
		}
		else
		{ // --get
			if((argc > 1) && (std::strcmp(argv[1], "--get") == 0))
				syslog(LOG_INFO, " ~~~ GET command");
			else
				syslog(LOG_ERR, "Invalid argument <%s>", argv[1]);

			json jout;
			fill_net_info(jout);
			// std::cout << "{\"mac\":\"aa-bb-cc-dd-ee-ff\",\"hostname\":\"testtest\",\"dhcp\":\"yes\",\"addr\":\"10.10.0.222\",\"mask\":\"255.255.255.0\",\"gateway\":\"10.10.0.1\",\"dns\":\"\",\"ip6_addr\":\"\",\"ip6_com\":\"add\",\"ip6_auto_conf\":\"on\",\"ip6_dhcp\":\"stateless\"}";
			std::cout << jout;
		}
	}
	else
	{
		struct ifaddrs *ifaddr, *ifa;
		int family, s, n;
		char host[NI_MAXHOST];

		gethostname(host, NI_MAXHOST);
		std::cout << "Hostname: " << host << std::endl;

		std::string gwstr;
		if(GetDefaultGw(gwstr) == 0)
			std::cout << "Default gateway: " << gwstr << std::endl;

		if (getifaddrs(&ifaddr) == -1) 
		{
			perror("getifaddrs");
			exit(EXIT_FAILURE);
		}

		/* Walk through linked list, maintaining head pointer so we can free list later */
		for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) 
		{
			if (ifa->ifa_addr == NULL)
				continue;

			family = ifa->ifa_addr->sa_family;
			if(family == AF_INET)
			{
				s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if(s != 0)
					std::cout << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
				else
					std::cout << ifa->ifa_name << "\taddress: <" << host << ">" << std::endl;

				s = getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if(s != 0)
					std::cout << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
				else
					std::cout << ifa->ifa_name << "\tnetmask: <" << host << ">" << std::endl;
			}
			else if(family == AF_INET6)
			{
				s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if(s != 0)
					std::cout << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
				else
					std::cout << ifa->ifa_name << "\taddress: <" << host << ">" << std::endl;

				s = getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if(s != 0)
					std::cout << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
				else
					std::cout << ifa->ifa_name << "\tnetmask: <" << host << ">" << std::endl;
			}
			else if (family == AF_PACKET)
			{
				struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
				// sockaddr_ll *s = (sockaddr_ll*)ifa->ifa_addr;
				printf("%-8s ", ifa->ifa_name);
				for (int i=0; i < s->sll_halen; i++)
				{
					printf("%02x%c", (s->sll_addr[i]), (i+1!=s->sll_halen)?':':'\n');
				}
            }

		}
		freeifaddrs(ifaddr);
	}
	return 0;
}


