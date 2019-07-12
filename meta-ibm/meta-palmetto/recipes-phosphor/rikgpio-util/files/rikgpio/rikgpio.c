//#include "gpio.h"
#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <dirent.h>

#include <argp.h>
#include <sys/stat.h>

#include <sys/ioctl.h>
#include <linux/gpio.h>

// #include <openbmc/gpio.h>

#include "gpio_name.h"

#define GPIO_PORT_OFFSET 8
#define GPIO_BASE_PATH "/sys/class/gpio"

#define GPIO_BASE_AA0  208
// #define DEBUG_MODE

/********************************************************************************************/
int get_gpio_base();
int gpio_num(char *str);		//Primitive
int gpio_export(int gpio);
int gpio_set_direction(short dir, int gpionum);
int gpio_set_value(short val, int gpionum);

/********************************************************************************************/
//static char *gpio_rikor[3] = {"GPIOQ7", "GPIOQ4", "GPIOY3"};
static char *gpio_rikor[36] = {FM_SLPS4, FM_SLPS3, FM_MEM_THERM_EVENT, FM_PCH_BMC_THERMTRIP, FM_CPU_CATERR, FM_CPU_ERR2, IRQ_SERIAL, FM_BMC_MIC_MUX_RST, FM_BMC_PCH_SMI_LPC, SPEAKER_BMC, FP_ID_LED, 
  FP_ID_BTN, FP_NMI_BTN, FM_BMC_PWRBTN_OUT, FM_BMC_PWR_BTN, FM_NMI_EVENT, FM_CPU1_MEMGH_MEMHOT, FM_CPU1_MEMEF_MEMHOT, FM_CPU0_MEMCD_MEMHOT, FM_CPU0_MEMAB_MEMHOT, FM_BMC_ONCTL, FP_LED_STATUS_AMBER, 
  FM_INTRUDER_HDR, FM_BMC_PCH_SCI_LPC, FP_LED_STATUS_GREEN, FM_PS_PWROK, FM_BMC_FLASH_WP, FM_BIOS_POST_CMPL, IRQ_SML0_ALERT_N, FM_SMI_ACTIVE, FM_FORCE_BMC_UPDATE, FM_CPU1_DISABLE, FP_PWR_BTN_MUX_N,
  FM_BMC_PWR_BTN_N, RST_BMC_RSTBTN_OUT_N, FP_RST_BTN_N};

static short gpio_dir[36] = {FM_SLPS4_DIR, FM_SLPS3_DIR, FM_MEM_THERM_EVENT_DIR, FM_PCH_BMC_THERMTRIP_DIR, FM_CPU_CATERR_DIR, FM_CPU_ERR2_DIR, IRQ_SERIAL_DIR, FM_BMC_MIC_MUX_RST_DIR, 
  FM_BMC_PCH_SMI_LPC_DIR, SPEAKER_BMC_DIR, FP_ID_LED_DIR, FP_ID_BTN_DIR, FP_NMI_BTN_DIR, FM_BMC_PWRBTN_OUT_DIR, FM_BMC_PWR_BTN_DIR, FM_NMI_EVENT_DIR, FM_CPU1_MEMGH_MEMHOT_DIR, FM_CPU1_MEMEF_MEMHOT_DIR, 
  FM_CPU0_MEMCD_MEMHOT_DIR, FM_CPU0_MEMAB_MEMHOT_DIR, FM_BMC_ONCTL_DIR, FP_LED_STATUS_AMBER_DIR, FM_INTRUDER_HDR_DIR, FM_BMC_PCH_SCI_LPC_DIR, FP_LED_STATUS_GREEN_DIR, FM_PS_PWROK_DIR, 
  FM_BMC_FLASH_WP_DIR, FM_BIOS_POST_CMPL_DIR, IRQ_SML0_ALERT_N_DIR, FM_SMI_ACTIVE_DIR, FM_FORCE_BMC_UPDATE_DIR, FM_CPU1_DISABLE_DIR, FP_PWR_BTN_MUX_N_DIR, FM_BMC_PWR_BTN_N_DIR, 
  RST_BMC_RSTBTN_OUT_N_DIR, FP_RST_BTN_N_DIR};

int gpio_rikor_num[36] = {};
/********************************************************************************************/

int main(void)
{
  int tmp = 0;

  int gpio_base = get_gpio_base();

  printf("Well, let's start...\n");
  for (int i=0; i<36; i++)
  {
    tmp = gpio_num(gpio_rikor[i]) + gpio_base;
    if (tmp < 0)
    {
#ifdef DEBUG_MODE
      printf("Failed to enum %s port\n", gpio_rikor[i]);
#endif	//DEBUG_MODE
      gpio_rikor_num[i] = 0;
    }
    else
    {
      gpio_rikor_num[i] = tmp;
#ifdef DEBUG_MODE
      printf("Num is %d \n", (int)tmp);
#endif	//DEBUG_MODE
    }    
  }
  
  for (int i=0; i<36; i++)
  {
    tmp = gpio_export(gpio_rikor_num[i]);
  }


  for (int i=0; i<36; i++)
  {
    tmp = gpio_set_direction(gpio_dir[i], gpio_rikor_num[i]);
    //For some outputs
  }
  
  gpio_set_value(1, gpio_num(FP_ID_LED) + gpio_base);
  gpio_set_value(1, gpio_num(FP_LED_STATUS_AMBER) + gpio_base);
  gpio_set_value(1, gpio_num(FP_LED_STATUS_GREEN) + gpio_base);


  system("/usr/bin/ledblink-1.0 135 10 &");             //135 - AMBER led gpio

  printf("Complete...\n");

  return 0;
}

/**
 * Determine the GPIO base number for the system.  It is found in
 * the 'base' file in the /sys/class/gpio/gpiochipX/ directory where the
 * /sys/class/gpio/gpiochipX/label file has a '1e780000.gpio' in it.
 *
 * Note: This method is ASPEED specific.  Could add support for
 * additional SOCs in the future.
 *
 * @return int - the GPIO base number, or < 0 if not found
 */
int get_gpio_base()
{
  int gpio_base = -1;

  DIR* dir = opendir(GPIO_BASE_PATH);
  if (dir == NULL)
  {
    fprintf(stderr, "Unable to open directory %s\n",
        GPIO_BASE_PATH);
    return -1;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != NULL)
  {
    /* Look in the gpiochip<X> directories for a file called 'label' */
    /* that contains '1e780000.gpio', then in that directory read */
    /* the GPIO base out of the 'base' file. */

    if (strncmp(entry->d_name, "gpiochip", 8) != 0)
    {
      continue;
    }

    //gboolean is_bmc = FALSE;
    int is_bmc = 0;
    char* label_name;
    asprintf(&label_name, "%s/%s/label",
        GPIO_BASE_PATH, entry->d_name);

    FILE* fd = fopen(label_name, "r");
    free(label_name);

    if (!fd)
    {
      continue;
    }

    char label[14];
    if (fgets(label, 14, fd) != NULL)
    {
      if (strcmp(label, "1e780000.gpio") == 0)
      {
        //is_bmc = TRUE;
	is_bmc = 1;
      }
    }
    fclose(fd);

    if (!is_bmc)
    {
      continue;
    }

    char* base_name;
    asprintf(&base_name, "%s/%s/base",
        GPIO_BASE_PATH, entry->d_name);

    fd = fopen(base_name, "r");
    free(base_name);

    if (!fd)
    {
      continue;
    }

    if (fscanf(fd, "%d", &gpio_base) != 1)
    {
      gpio_base = -1;
    }
    fclose(fd);

    /* We found the right file. No need to continue. */
    break;
  }
  closedir(dir);

  if (gpio_base == -1)
  {
    fprintf(stderr, "Could not find GPIO base\n");
  }

  return gpio_base;
}


/********************************************************************************************/
int gpio_num(char *str)
{
  int len = strlen(str);
  int ret = 0;

  if (len != 6 && len != 7) {
    return -1;
  }
  ret = str[len-1] - '0' + (8 * (str[len-2] - 'A'));
  if (len == 7)
    ret += GPIO_BASE_AA0;
  return ret;
}

/********************************************************************************************/
int gpio_export(int gpio)
{
  char buf[128] = {0};
  int fd = -1;
  int rc = 0;
  int len;

  snprintf(buf, sizeof(buf), "/sys/class/gpio/export");
  fd = open(buf, O_WRONLY);
  if (fd == -1) {
    rc = errno;
    printf("Failed to export %d gpioport...\n", (int)gpio);
    return -rc;
  }

  len = snprintf(buf, sizeof(buf), "%d", gpio);
  write(fd, buf, len);
  close(fd);

  return rc;
}

/********************************************************************************************/
int gpio_set_direction(short dir, int gpionum)
{
  char buf[128];
  int fd = -1;
  int rc = 0;
  char *val;

  snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", (int)gpionum);
  fd = open(buf, O_WRONLY);
  if (fd == -1) {
    rc = errno;
    return -rc;
  }

  if (dir == 0) val = "in";
  else val = "out";

  write(fd, val, strlen(val));

  if (fd != -1) {
    close(fd);
  }
  return -rc;
}



/********************************************************************************************/
int gpio_set_value(short val, int gpionum)
{
  char buf[128];
  int fd = -1;
  int rc = 0;
  char *outstr;

  snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpionum);
  fd = open(buf, O_WRONLY);
  if (fd == -1) {
    rc = errno;
    return -rc;
  }

  if (val != 0) outstr = "1";
  else outstr = "0";

  write(fd, outstr, strlen(outstr));

  if (fd != -1) {
    close(fd);
  }
  return -rc;
}
