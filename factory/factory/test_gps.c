#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <sys/time.h>

#include "common.h"
#include "config_parser.h"
#include "packet.h"
#include "test_item.h"
#include <errno.h>

#define CONFIG_BAUDRATE	115200
#define CONFIG_DATABIT	8
#define CONFIG_EVENT	'N'
#define CONFIG_STOPBIT	1

static pthread_t gAtePid = 0;

static int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio, oldtio;

	if (tcgetattr(fd,&oldtio) != 0) {
		perror("SetupSerial 1");
		return -1;
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch(nBits) {
	case 7:
		newtio.c_cflag |= CS7;
		break;
	case 8:
		newtio.c_cflag |= CS8;
		break;
	}

	switch(nEvent) {
	case 'O':
		newtio.c_cflag |= PARENB;
		newtio.c_cflag |= PARODD; 
		newtio.c_iflag |= (INPCK | ISTRIP);
		break;
	case 'E':
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;
		break;
	case 'N':
		newtio.c_cflag &= ~PARENB;
		break;
	}

	switch(nSpeed) {
	case 2400:
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	case 460800:
		cfsetispeed(&newtio, B460800);
		cfsetospeed(&newtio, B460800);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}

	if (nStop == 1) {
		newtio.c_cflag &=  ~CSTOPB;
	} else if (nStop == 2) {
		newtio.c_cflag |=  CSTOPB;
	}


	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd, TCIFLUSH); 
	if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
		perror("com set error");
		return -1;
	}
         
	return 0;
}

int set_flowcontrol(int fd, int control) 
{
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) 
    {
        perror("error from tggetattr");
        return -1;
    }

    if(control) tty.c_cflag |= CRTSCTS;
    else tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("error setting term attributes");
        return -1;
    }
    return 0;
}

static int check_gps(int fd, int timeout, char *retString)
{
	int ret;
	char buf[1024] = {0};

	if (!retString) {
		return -1;
	}

#if  0
	ret = set_opt(fd, CONFIG_BAUDRATE, CONFIG_DATABIT,
				CONFIG_EVENT, CONFIG_STOPBIT);
	if (ret < 0) {
		LOGE("set_opt failed.\n");
		sprintf(retString, "NG:config_failed");
		return -2;
	}
#endif

	struct timeval tv;
	gettimeofday(&tv, NULL);
	long start =  tv.tv_sec;
    
	while (1) {
		gettimeofday(&tv, NULL);
		long current = tv.tv_sec;
		if ((current - start) >= timeout)
			break;

		struct timeval timeout; //timeout 2s
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		fd_set rd;
		FD_ZERO(&rd);
		FD_SET(fd, &rd);
		memset(buf, 0, sizeof(buf));

		ret = select(fd + 1, &rd, NULL, NULL, &timeout);
		if (ret < 0) {
			perror("gps select read");
			continue;
		}

		if (ret == 0) {
			//timeout
			continue;
		}

		ret = read(fd, buf, sizeof(buf));
		if (ret < 0) {
			LOGE("read data failed.\n");
			sprintf(retString, "NG:read_data_%d", ret);
			return -2;
		}

		if (ret == 0)
			continue;

		char *ant = strstr(buf, "ANTENNA");
		if (ant) {
			char *antStatus = ant + strlen("ANTENNA ");

			if (strncmp(antStatus, "OK", 2) == 0) {
				sprintf(retString, "OK");
			} else if (strncmp(antStatus, "OPEN", 2) == 0) {
				sprintf(retString, "NG;antenna_open");
			} else if (strncmp(antStatus, "SHORT", 2) == 0) {
				sprintf(retString, "NG;antenna_short");
			}

			return 0;
		}
	}

	LOGE("wait for response data timeout.\n");
	sprintf(retString, "NG;TimeOut");
	return -1;
}

void *do_gps_test(void *param)
{
	int ret;
	int timeout = 5;
        int nread;
	struct packet_info info;

	char itemName[32] = {0};
	char devName[32] = {0};
	char devPath[64] = {0};
	char retString[32] = {0};

	LOGV("%s\n", __func__);
	memcpy(&info, param, sizeof(struct packet_info));

	if (info.unit_id < 0) {
		LOGE("invalid gps id%d\n", info.unit_id);
		sprintf(retString, "NG:Invalid_id");
		send_response_packet(&info, retString, strlen(retString));
		return NULL;
	}
		
	timeout = info.timeout;
	LOGV("timeout value %d\n", timeout);

	sprintf(itemName, "gps%d", info.unit_id);
	ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
			itemName, "device", devName);
	if (ret != 0) {
		LOGW("attribute device not found, assume ttyWCHUSB1\n");
		sprintf(devName, "ttyWCHUSB1");
	}

	sprintf(devPath, "/dev/%s", devName);

	int fd = open(devPath, O_RDONLY);
	if (fd < 0) {
		LOGE("open %s failed: %s.\n", devPath, strerror(errno));
		sprintf(retString, "NG;open_failed");
		send_response_packet(&info, retString, strlen(retString));
		return NULL;
	}

	check_gps(fd, timeout, retString);
	set_flowcontrol(fd, 0);

        close(fd);
	send_response_packet(&info, retString, strlen(retString));

	return NULL;
}
