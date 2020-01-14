
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>


static int gpio_base = -1;

#define GPIO_PORT_OFFSET 8
#define GPIO_BASE_PATH "/sys/class/gpio"

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
	int retval = -1;

	DIR* dir = opendir(GPIO_BASE_PATH);
	if (dir == NULL)
	{
		fprintf(stderr, "Unable to open directory %s\n", GPIO_BASE_PATH);
		return -1;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		/* Look in the gpiochip<X> directories for a file called 'label' */
		/* that contains '1e780000.gpio', then in that directory read */
		/* the GPIO base out of the 'base' file. */

		if (strncmp(entry->d_name, "gpiochip", 8) != 0)
			continue;

		//gboolean is_bmc = FALSE;
		int is_bmc = 0;
		char* label_name;
		asprintf(&label_name, "%s/%s/label",
		         GPIO_BASE_PATH, entry->d_name);

		FILE* fd = fopen(label_name, "r");
		free(label_name);

		if (!fd)
			continue;

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
			continue;

		char* base_name;
		asprintf(&base_name, "%s/%s/base",
		         GPIO_BASE_PATH, entry->d_name);

		fd = fopen(base_name, "r");
		free(base_name);

		if (!fd)
			continue;

		if (fscanf(fd, "%d", &retval) != 1)
			retval = -1;
		fclose(fd);

		/* We found the right file. No need to continue. */
		break;
	}
	closedir(dir);

	if (retval == -1)
		fprintf(stderr, "Could not find GPIO base\n");

	return retval;
}



#define GPIO_BASE_AA0  208

int
gpio_num(char *str)
{
	if (gpio_base == -1)
		gpio_base = get_gpio_base();

	int len = strlen(str);
	int ret = 0;

	if (len != 6 && len != 7) {
		return -1;
	}
	ret = str[len - 1] - '0' + (8 * (str[len - 2] - 'A'));
	if (len == 7)
		ret += GPIO_BASE_AA0;
	return ret + gpio_base;
}



char *
gpio_name(int gpio, char *str)
{
	return NULL;
}

