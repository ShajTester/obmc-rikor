

#include <iostream>
#include <fstream>
#include <string>
#include <array>

#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "json.hpp"
using json = nlohmann::json;


struct sensor
{
	int num;
	std::string name;
	int r1;
	int r2;
	int v2;
};


sensor sensors_data[] = 
{
	{ 0, "P3V3_BASE",      5100,  8200, 0},
	{ 1, "P12V_BASE",      5110,  1000, 0},
	{ 2, "P3V3_AUX",       5100,  8200, 0},
	{ 3, "P5V_BASE",       5100,  3400, 0},
	{ 4, "P1V05_STBY_PCH",    0,     1, 0},
	{ 5, "P1V05_PCH",         0,     1, 0},
	{ 6, "P1V5_STBY_PCH",     0,     1, 0},
	{ 7, "PVCCIO",            0,     1, 0},
	{ 8, "DRAM_PVPP",      5110, 20500, 0},
	{ 9, "DRAM_GH",           0,     1, 0},
	{10, "DRAM_EF",           0,     1, 0},
	{11, "DRAM_CD",           0,     1, 0},
	{12, "DRAM_AB",           0,     1, 0},
	{13, "CPU1_PVCCIN",       0,     1, 0},
	{14, "CPU0_PVCCIN",       0,     1, 0}
};

#define sizeof_sensors_data (sizeof(sensors_data) / sizeof(sensor))

#define ADC_RAW_PATH  "/sys/bus/platform/drivers/aspeed_adc/1e6e9000.adc/iio:device0/in_voltage%d_raw"

int main(int argc, char const *argv[])
{
    openlog("rikcgi-voltagesensors", LOG_CONS, LOG_USER);
    // syslog(LOG_INFO, "rikcgi-voltagesensors: started");
	
    std::array<std::array<std::string, 2>, sizeof_sensors_data> outdata;

	char fname[128];
	char sval[32];
	std::ifstream f;
	unsigned int val;
	unsigned int voltage, tmp1, tmp2, tmp3;

	for (int i = 0; i < sizeof_sensors_data; i++)
	{
		sprintf(fname, ADC_RAW_PATH, sensors_data[i].num);
		f.open(fname);
		f >> val;
		f.close();
		outdata[i][0] = sensors_data[i].name;

		tmp1 = (sensors_data[i].r1 + sensors_data[i].r2) * val * 25 * 10;
		tmp2 = sensors_data[i].r2 * 1024;
		tmp3 = (sensors_data[i].r1 * sensors_data[i].v2) / sensors_data[i].r2;
		voltage = (tmp1 / tmp2) - tmp3;

		sprintf(sval, "%g V", static_cast<float>(voltage) / 100.0);
		outdata[i][1] = sval;
	}

	json jout(outdata);
	std::cout << jout << std::endl;

	return 0;
}


