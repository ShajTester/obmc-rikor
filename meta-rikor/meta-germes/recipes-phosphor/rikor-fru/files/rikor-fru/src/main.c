/*
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 * Contributor(s): Jiri Hnidek <jiri.hnidek@tul.cz>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>

#include <stdarg.h>
#include <limits.h>

#include "rikor-fru.h"

#include <arpa/inet.h>


static char *conf_file_name = NULL;
static char *app_name = NULL;

// http://digitalchip.ru/osobennosti-ispolzovaniya-extern-i-static-v-c-c
// Однако иногда бывает нужно, чтобы глобальная переменная или функция 
// были видны только в том файле, где определены. 
// Это позволяет сделать атрибут static
FILE *log_stream;
FILE *text_result;



in_addr_t parse_ip(const char *str)
{
	return inet_addr(str);
}


/**
 * \brief Print help for this application
 */
void print_help(void)
{
	printf("Usage: %s [OPTIONS]\n\n", app_name);
	printf("  Options:\n");
	printf("   -h --help                 Print this help\n");
	printf("   -g --get='<param list>'   Get data\n");
	printf("   -s --set='<param list>'   Store data\n");
	printf("   -a --addr xx              FRU address. Between 0x50, 0x57.\n");
	printf("   -b --faddr path           Path to the file with the FRU address.\n");
	printf("\n");
}


void printf_ip(struct in_addr ip)
{
	fputs(inet_ntoa(ip), stdout);
}


void print_param(const char *start, const char *end, const rikor_fru_t *data)
{
	if(strncmp(start, "dhcp1", (end - start)) == 0)
	{
		if(data->dhcp1 == 1) printf("dhcp");
		else printf("static");
	}
	else if(strncmp(start, "ip1", (end - start)) == 0)
	{
		printf_ip(data->ip1);
	}
	else if(strncmp(start, "netmask1", (end - start)) == 0)
	{
		printf_ip(data->netmask1);
	}
	else if(strncmp(start, "gate1", (end - start)) == 0)
	{
		printf_ip(data->gate1);
	}
	else if(strncmp(start, "dhcp2", (end - start)) == 0)
	{
		if(data->dhcp2 == 1) printf("dhcp");
		else printf("static");
	}
	else if(strncmp(start, "ip2", (end - start)) == 0)
	{
		printf_ip(data->ip2);
	}
	else if(strncmp(start, "netmask2", (end - start)) == 0)
	{
		printf_ip(data->netmask2);
	}
	else if(strncmp(start, "gate2", (end - start)) == 0)
	{
		printf_ip(data->gate2);
	}
	else if(strncmp(start, "hostname", (end - start)) == 0)
	{
		printf("%s", data->hostname);
	}
	else
	{
		for(const char *i=start; i<end; i++)
			putchar(*i);
		putchar('\n');
	}
}


void print_list(const char *line, const rikor_fru_t *data)
{
	const char *c = line;
	const char *start = line;
	for(; *c != 0; c++)
	{
		if(*c == ' ')
		{
			print_param(start, c, data);
			putchar(' ');
			start = c + 1;
		}
	}
	if(start != c) print_param(start, c, data);
}


int read_param(const char *start, const char *mid, char *end, rikor_fru_t *data)
{
	int retval = 0;
	struct in_addr ip;
	char tc;

	syslog(LOG_INFO, " ~ %p  %p  %p", start, mid, end);

	if(strncmp(start, "dhcp1", (mid - start - 1)) == 0)
	{
		if(strncmp(mid, "yes", (end - mid)) == 0) data->dhcp1 = 1;
		else if(strncmp(mid, "no", (end - mid)) == 0) data->dhcp1 = 0;
		else
		{
			retval++;
			syslog(LOG_ERR, "Read param dhcp1 data error");
		}
	}
	else if(strncmp(start, "ip1", (mid - start - 1)) == 0)
	{
		tc = *end;
		*end = 0;
		ip.s_addr = parse_ip(mid);
		*end = tc;
		if(ip.s_addr != 0) data->ip1 = ip;
		else
		{
			retval++;
			syslog(LOG_ERR, "Read param ip1 data error");
		}
	}
	else if(strncmp(start, "netmask1", (mid - start - 1)) == 0)
	{
		tc = *end;
		*end = 0;
		ip.s_addr = parse_ip(mid);
		*end = tc;
		if(ip.s_addr != 0) data->netmask1 = ip;
		else
		{
			retval++;
			syslog(LOG_ERR, "Read param netmask1 data error");
		}
	}
	else if(strncmp(start, "gate1", (mid - start - 1)) == 0)
	{
		tc = *end;
		*end = 0;
		ip.s_addr = parse_ip(mid);
		*end = tc;
		if(ip.s_addr != 0) data->gate1 = ip;
		else
		{
			retval++;
			syslog(LOG_ERR, "Read param gate1 data error");
		}
	}
	else if(strncmp(start, "dhcp2", (mid - start - 1)) == 0)
	{
		if(strncmp(mid, "yes", (end - mid)) == 0) data->dhcp2 = 1;
		else if(strncmp(mid, "no", (end - mid)) == 0) data->dhcp2 = 0;
		else
		{
			retval++;
			syslog(LOG_ERR, "Read param dhcp2 data error");
		}
	}
	else if(strncmp(start, "ip2", (mid - start - 1)) == 0)
	{
		tc = *end;
		*end = 0;
		ip.s_addr = parse_ip(mid);
		*end = tc;
		if(ip.s_addr != 0) data->ip2 = ip;
		else
		{
			retval++;
			syslog(LOG_ERR, "Read param ip2 data error");
		}
	}
	else if(strncmp(start, "netmask2", (mid - start - 1)) == 0)
	{
		tc = *end;
		*end = 0;
		ip.s_addr = parse_ip(mid);
		*end = tc;
		if(ip.s_addr != 0) data->netmask2 = ip;
		else
		{
			retval++;
			syslog(LOG_ERR, "Read param netmask2 data error");
		}
	}
	else if(strncmp(start, "gate2", (mid - start - 1)) == 0)
	{
		tc = *end;
		*end = 0;
		ip.s_addr = parse_ip(mid);
		*end = tc;
		if(ip.s_addr != 0) data->gate2 = ip;
		else
		{
			retval++;
			syslog(LOG_ERR, "Read param gate2 data error");
		}
	}
	else if(strncmp(start, "hostname", (mid - start - 1)) == 0)
	{
		strncpy(data->hostname, mid, end - mid);
		*(data->hostname + (end - mid)) = 0;
	}
	else
	{
		for(const char *i=start; i<end; i++)
			putchar(*i);
		putchar('\n');
		retval++;
	}

	return 1;
}


int read_list(char *line, rikor_fru_t *data)
{
	char *c = line;
	const char *start = line;
	const char *mid = line;
	int retval = 0;

	for(; *c != 0; c++)
	{
		if(*c == ' ')
		{
			if(mid <= start)
			{
				mid = c + 1;
			}
			else
			{
				retval += read_param(start, mid, c, data);
				start = c + 1;
			}
		}
	}
	if((start < mid) && (mid < c)) read_param(start, mid, c, data);
	return retval;
}


/* Main function */
int main(int argc, char *argv[])
{
	int retval = -1;
	// Инициализация

	static struct option long_options[] = 
	{
		{"help",  no_argument,       0, 'h'},
		{"set",   required_argument, 0, 's'},
		{"get",   optional_argument, 0, 'g'},
		{"addr",  required_argument, 0, 'a'},
		{"faddr", required_argument, 0, 'b'},
		{"initdata", no_argument,    0, 'z'},
		{NULL, 0, 0, 0}
	};

	int value;
	int option_index = 0;
	char *ip_string = NULL;
	char *get_string = NULL;
	int func = 0;
	int at24addr = 0x50;

	app_name = argv[0];

    openlog("rikor-fru", LOG_CONS, LOG_USER);


	/* Try to process all command line arguments */
	while ((value = getopt_long(argc, argv, "hs:g::a:b:", long_options, &option_index)) != -1) 
	{
		switch (value) 
		{
			case 's':
				func = 1;
				get_string = strdup(optarg);
				break;
			case 'g':
				func = 2;
				if(optarg != NULL)
				{
					get_string = strdup(optarg);
				}
				break;
			case 'a':
				sscanf(optarg, "%x", &at24addr);
				if((at24addr < 0x50) || (at24addr > 0x57))
				{
					syslog(LOG_ERR, "Wrong AT24C02 address %d\n", at24addr);
					at24addr = 0x50;
				}
				break;
			case 'b':
			{
				FILE *ffaddr = fopen(optarg, "rb");
				if(ffaddr == NULL)
				{
					syslog(LOG_ERR, "Can not open file: %s, error: %s\n",
						optarg, strerror(errno));
				}
				else
				{
					fscanf(ffaddr, "%x", &at24addr);
					if((at24addr < 0x50) || (at24addr > 0x57))
					{
						syslog(LOG_ERR, "Wrong AT24C02 address %d\n", at24addr);
						at24addr = 0x50;
					}
				}
				break;
			}
			case 'z':
				func = 3;
				break;
			case '0':
			case '1':
				syslog(LOG_ERR, "NUMBER %s: option '-%c' is invalid: ignored\n", argv[0], optopt);
				break;
			case 'h':
			case '?':
				print_help();
				break;
			case ':':
			default:
				syslog(LOG_ERR, "%s: option '-%c' is invalid: ignored\n", argv[0], optopt);
				break;
		}
	}



	char eeprom_path[256];
	rikor_fru_t data;
	int rf;

	sprintf(eeprom_path, EEPROM_PATH, at24addr);

	if((rf = fru_buf_init(&data)) == 0)
	{

		switch(func)
		{
		case 1:
			// Store data
			rf = read_fru(eeprom_path, &data);
			if(rf == ERRCRC)
			{ // Из EEPROM прочитаны неправильные данные
				syslog(LOG_ERR, "EEPROM CRC error");
				fru_buf_init(&data);
				retval = 1;
			}
			else if(rf != 0)
			{ // Данные успешно прочитаны
				syslog(LOG_ERR, "EEPROM read error %d", rf);
				retval = 1;
				// Если eeprom прочитать не смогли, то записать точно не сможем
				break;
			}

			rf = read_list(get_string, &data);
			if(write_fru(eeprom_path, &data) != 0)
			{
				syslog(LOG_ERR, "EEPROM write error");
				retval = 1;
			}
			break;
		case 2:
			// Get IP
			rf = read_fru(eeprom_path, &data);
			if(rf == ERRCRC)
			{
				syslog(LOG_ERR, "EEPROM CRC error");
				fru_buf_init(&data);
				retval = 1;
			}
			else if(rf != 0)
			{
				syslog(LOG_ERR, "EEPROM read error %d", rf);
				fru_buf_init(&data);
				retval = 1;
			}

			if(get_string == NULL)
			{
				print_list("dhcp1 ip1 netmask1 gate1 dhcp2 ip2 netmask2 gate2 hostname", &data);
				putchar('\n');
			}
			else
			{
				print_list(get_string, &data);
				putchar('\n');
			}
			break;
		case 3:
			// Инициализация eeprom данными по умолчанию
			rf = read_fru(eeprom_path, &data);
			if(rf == ERRCRC)
			{
				rf = fru_buf_init(&data);
				if(write_fru(eeprom_path, &data) != 0)
				{
					syslog(LOG_ERR, "EEPROM write error");
					retval = 1;
				}
			}
			else
			{
				syslog(LOG_ERR, "The correct data has already been written to the EEPROM.");
				retval = 1;
			}
			break;
		default:
			// Get all FRU data
			rf = read_fru(eeprom_path, &data);
			if(rf == ERRCRC)
			{
				syslog(LOG_ERR, "EEPROM CRC error\n");
				fru_buf_init(&data);
				retval = 1;
			}
			else if(rf != 0)
			{
				syslog(LOG_ERR, "EEPROM read error %d\n", rf);
				fru_buf_init(&data);
				retval = 1;
			}

			printf("FRU id:   0x%08X\n", data.id);
			printf("Board id: 0x%016llX\n\n", data.board_id);
			if(data.dhcp1 == 1) printf("if1 DINAMIC\n");
			else printf("if1 STATIC\n");
			printf("ip1:       %s\n", inet_ntoa(data.ip1));
			printf("netmask1:  %s\n", inet_ntoa(data.netmask1));
			printf("gate1:     %s\n", inet_ntoa(data.gate1));
			if(data.dhcp2 == 1) printf("if2 DINAMIC\n");
			else printf("if2 STATIC\n");
			printf("ip2:       %s\n", inet_ntoa(data.ip2));
			printf("netmask2:  %s\n", inet_ntoa(data.netmask2));
			printf("gate2:     %s\n", inet_ntoa(data.gate2));

			printf("Hostname: <%s>\n", data.hostname);
			data.psw1[data.psw1size] = 0;
			printf("psw1:     <%s>\n", data.psw1);
			data.psw2[data.psw2size] = 0;
			printf("psw2:     <%s>\n", data.psw2);
			retval = 0;
			break;
		}
	}
	else
	{
		syslog(LOG_ERR, "error in fru_buf_init()\n");
		retval = 2;
	}




	/* Close log file, when it is used. */
	// if (log_stream != stdout) {
	// 	fclose(log_stream);
	// }

	/* Free allocated memory */
	if(ip_string != NULL) free(ip_string);
	if(get_string != NULL) free(get_string);

	/* Free allocated memory */

	return retval;
}
