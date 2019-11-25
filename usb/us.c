// SPDX-License-Identifier: GPL-2.0
/*
 * User space application to demonstrate interaction with example
 * linux USB driver which interacts with custom firmware running
 * on STM32F4Discovery development board.
 *
 * Copyright (C) 2017 Roman Okhrimenko <mrromanjoe@gmail.com>
 * 
 * Description on this application, driver and
 * STM32F4Discovery firmware can be found here
 * <https://github.com/romanjoe/os-course-labs/usb>
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define blue			0x8
#define red 			0x4
#define orange			0x2
#define green 			0x1
#define leave_colors	0xFF

static int send_data(int fd, char * data)
{
	int retval = 0;

	retval = write(fd, data, 10);
	if (retval < 0)
		fprintf(stderr, "could not send command to fd=%d\n", fd);

	return retval;
}

static void receive_data(int fd)
{
	char buf[10];
	int retval = 0;
	int i;

	retval = read(fd, buf, 10);
	if(retval < 0)
	{
		fprintf(stderr, "could not read from fd = %d\n", fd);
	}
	else
	{
		printf("Blue button was pressed %d times\n", buf[1]);
	}
}

static void help(char *app_name)
{
	fprintf(stderr,
					"\nUsage: %s [-rgboafsh]\n\n" 
					"-r - red light LED5\n"
					"-g - green light LED4\n"
					"-b - blue light LED6\n"
					"-o - orange light LED3\n"
					"-a - all lights are on\n"
					"-f - all lights are off\n"
					"-s - button press count\n"
					"-h - help\n\n"
					"example usage:\n"
					"-rgb - red, greed, blue are on; orange is off\n"
					"-rb  - red, blue are on; orange, green are off\n",
					app_name

					);
}

int main(int argc, char *argv[])
{
	int fd;
	int retval;
	int c;
	char color_combination = 0;
	char get_button_counts = 0;

	fd = open("/dev/stm32leds0", O_RDWR);
	if (fd == -1) {
			perror("open");
			exit(1);
	}

	while((c = getopt(argc, argv, "rgboafsh:")) != -1)
	{
		switch(c)
		{
			case 'r':
				color_combination |= red;
				break;
			case 'b':
				color_combination |= blue;
				break;
			case 'g':
				color_combination |= green;
				break;
			case 'o':
				color_combination |= orange;
				break;
			case 'a':
				color_combination |= orange | green | red | blue;
				break;
			case 'f':
				color_combination = 0;
				break;
			case 's':
				color_combination = leave_colors;
				get_button_counts = 1;
				break;
			case 'h':
				help(argv[0]);
				break;
			default:
				help(argv[0]);
		}
	}
	char data[10] = {1,0,0,0,0,0,0,0,get_button_counts,color_combination};

	retval = send_data(fd, data);

	if(retval >= 0 && get_button_counts == 1)
	{
		receive_data(fd);
	}

	close(fd);

	return 0;
}