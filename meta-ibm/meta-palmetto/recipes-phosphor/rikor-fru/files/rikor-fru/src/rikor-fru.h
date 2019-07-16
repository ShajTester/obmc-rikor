

#ifndef RIKOR_FRU_H
#define RIKOR_FRU_H

#include <stdbool.h>
#include <linux/types.h>
#include <arpa/inet.h>

#define DEFAT24ADDR 0x50
#define EEPROM_PATH "/sys/devices/platform/ast-i2c.3/i2c-3/3-00%02X/eeprom"

#define ERRCRC      -2

#ifdef __cplusplus
extern "C" {
#endif



#pragma pack(push, 1)
typedef struct 
{
	unsigned int id;
	unsigned long long board_id;
	union
	{
		struct
		{
			unsigned int dhcp1: 1;
			unsigned int notused1: 15;
			unsigned int dhcp2: 1;
			unsigned int notused2: 15;
		};
		unsigned int netflags;
	};
	struct in_addr ip1;
	struct in_addr netmask1;
	struct in_addr gate1;
	struct in_addr dns1_1;
	struct in_addr ip2;
	struct in_addr netmask2;
	struct in_addr gate2;
	struct in_addr dns1_2;
	char hostname[18];
	unsigned char psw1size;
	char psw1[17];
	unsigned char psw2size;
	char psw2[17];
} rikor_fru_t;
#pragma pack(pop)

typedef enum
{
	rikor_fru_psw1,
	rikor_fru_psw2
} rikor_fru_psw_t;


int get_fru_device(char *path);

int fru_buf_init(rikor_fru_t *data);
int read_fru(const char *device, rikor_fru_t *data);
int write_fru(const char *device, const rikor_fru_t *data);

bool check_psw(rikor_fru_psw_t psw, const char *str, const rikor_fru_t *data);
int set_psw(rikor_fru_psw_t psw, const char *oldPsw, const char *newPsw, rikor_fru_t *data);

/*
 * В state передается символ '0' или '1'
 * В кодах это 0x30 и 0x31. Т.е. младший бит соответствует состоянию,
 * а остальные являются проверкой записи.
 *
 * 0 - состояние отключено;
 * 1 - состояние включено.
 * 
 * При любой ошибке чтения eeprom, функция в state возвращает 0.
 *
 * Если для записи в state передается любой символ кроме '1',
 * то в EEPROM записывается '0'.
 *
 * Функция возвращает 0 или код ошибки
 */
int set_brd_state(const char *device, const char state);
int get_brd_state(const char *device, char *state);


#ifdef __cplusplus
}
#endif


#endif // RIKOR_FRU_H
