

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>

#include "crc16.h"
#include "rikor-fru.h"


// #ifndef DEBUG
// #define DEBUG
// #endif


// Отсюда
// https://github.com/KyleBanks/XOREncryption/blob/master/C%2B%2B/main.cpp
// https://toster.ru/q/489264
static char key[] = {'K', 'C', 'Q'}; //Any chars will work, in an array of any size

void encryptDecrypt(char *toEncrypt, int len)
{
    for (int i = 0; i < len; i++)
        toEncrypt[i] = toEncrypt[i] ^ key[i % (sizeof(key) / sizeof(char))];
}



int fru_buf_init(rikor_fru_t *data)
{
	data->id = 0xaa5555aa;
	data->board_id = 0x1234567890abcdefL;
	data->dhcp1 = 1;
	inet_aton("192.168.0.220", &data->ip1);
	inet_aton("255.255.255.0", &data->netmask1);
	inet_aton("192.168.0.1", &data->gate1);
	data->dhcp2 = 1;
	inet_aton("10.10.0.220", &data->ip2);
	inet_aton("255.255.255.0", &data->netmask2);
	inet_aton("10.10.0.1", &data->gate2);

	strcpy(data->hostname, "bmc-rikor");

	memset(data->psw1, 0, sizeof(data->psw1));
	data->psw1size = 4;
	strcpy(data->psw1, "1234");
	encryptDecrypt(data->psw1, data->psw1size);

	memset(data->psw2, 0, sizeof(data->psw2));
	data->psw2size = 6;
	strcpy(data->psw2, "qwerty");
	encryptDecrypt(data->psw2, data->psw2size);

	return EXIT_SUCCESS;
}


int get_fru_device(char *path)
{
	int retval = 0;
	int at24addr = DEFAT24ADDR;

	FILE *ffaddr = fopen("/tmp/rikor-fru-address", "rb");
	if(ffaddr == NULL)
	{
		syslog(LOG_ERR, "Can not open file: %s, error: %s\n", "/tmp/rikor-fru-address", strerror(errno));
		retval = 1;
	}
	else
	{
		fscanf(ffaddr, "%x", &at24addr);
		if((at24addr < 0x50) || (at24addr > 0x57))
		{
			syslog(LOG_ERR, "Wrong AT24C02 address %d\n", at24addr);
			at24addr = 0x50;
			retval = 2;
		}
	}

	sprintf(path, EEPROM_PATH, at24addr);
	return retval;
}


int read_fru(const char *device, rikor_fru_t *data)
{
	// unsigned char buf[256];
	FILE *fp;
	unsigned short crc;
	unsigned short fcrc;
	int rc;

	fp = fopen(device, "rb");
	if (!fp)
	{
		int err = errno;
		syslog(LOG_ERR, "failed to open device %s", device);
		return err;
	}

	rc = fread(data, sizeof(rikor_fru_t), 1, fp);
	rc += fread(&fcrc, 2, 1, fp);
	fclose(fp);

	if (rc != 2)
	{
		syslog(LOG_ERR, "failed to read device %s", device);
		return ENOENT;
	} 
	else
	{
		crc = crc16(0, (const u8 *)data, sizeof(rikor_fru_t));
		if(fcrc != crc)
		{ // Ошибка CRC
			syslog(LOG_ERR, "CRC error in device %s", device);
			return ERRCRC;
		}
		return 0;
	}

	// crc = crc16(0, (const u8 *)data, sizeof(rikor_fru_t));
	// fprintf(stderr, "crc16(0) %d\n", crc);
	// fprintf(stderr, "%d\n%d\n%d\n", (crc + (~crc) + 1), crc * (~crc), crc - ~crc);
	// return -1;
}



int write_fru(const char *device, const rikor_fru_t *data)
{
	FILE *fp;
	unsigned short crc;
	int rc;

	fp = fopen(device, "wb");
	if (!fp)
	{
		int err = errno;

		syslog(LOG_ERR, "failed to open device %s", device);
		return err;
	}

	rc = fwrite(data, sizeof(rikor_fru_t), 1, fp);
	crc = crc16(0, (const u8 *)data, sizeof(rikor_fru_t));
	rc += fwrite(&crc, 2, 1, fp);
	fclose(fp);

	return 0;
}



bool check_psw(rikor_fru_psw_t psw, const char *str, const rikor_fru_t *data)
{
	char dpsw[17];
	if(psw == rikor_fru_psw1)
	{
		strcpy(dpsw, data->psw1);
	}
	else if(psw == rikor_fru_psw2)
	{
		strcpy(dpsw, data->psw2);
	}
	else return false;

	encryptDecrypt(dpsw, strlen(dpsw));
	if(strcmp(dpsw, str) == 0) return true;

	return false;
}


int set_psw(rikor_fru_psw_t psw, const char *oldPsw, const char *newPsw, rikor_fru_t *data)
{
	char *dest;
	if(psw == rikor_fru_psw1)
		dest = data->psw1;
	else
		dest = data->psw2;

	if(strlen(newPsw) <= 16)
	{
		strcpy(dest, newPsw);
		encryptDecrypt(dest, strlen(newPsw));
	}
	else
	{
		strncpy(dest, newPsw, 16);
		dest[16] = 0;
		encryptDecrypt(dest, 16);
	}

	return 0;
}


int set_brd_state(const char *device, const char state)
{
	return -1;
}


int get_brd_state(const char *device, char *state)
{
	return -1;
}

