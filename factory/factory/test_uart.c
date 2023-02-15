/*
 * 串口测试：发送数据给ATE，接收ATE返回的数据，判断数据是否正确。
 */

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

#include "common.h"
#include "config_parser.h"
#include "packet.h"
#include "test_item.h"

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

static int test_tsinput(int fd, int timeout, char *retString)
{
	int ret;
	int loop;
	const char *retData = "78";
	char recvBuf[64] = {0};

	if (!retString) {
		return -1;
	}

	ret = set_opt(fd, CONFIG_BAUDRATE, CONFIG_DATABIT,
				CONFIG_EVENT, CONFIG_STOPBIT);
	if (ret < 0) {
		LOGE("set_opt failed.\n");
		sprintf(retString, "NG:config_failed");
		return -2;
	}

	for (loop = 0; loop < timeout; loop++) {
		ret = read(fd, recvBuf, sizeof(recvBuf));
		if (ret < 0) {
			LOGE("read data failed.\n");
			sprintf(retString, "NG:read_data_%d", ret);
			return -2;
		}

		if (ret > 0) {
			if (ret != (int)strlen(retData) ||
					strncmp(recvBuf, retData, strlen(retData))) {
				LOGE("receive invalid data [%s] %d.\n", recvBuf, ret);
				sprintf(retString, "NG:ret_data_error");
				return -2;
			}

			sprintf(retString, "OK");
			return 0;
		}

		sleep(1);
	}

	LOGE("wait for response data timeout.\n");
	sprintf(retString, "NG:TimeOut");
	return -1;
}

static int test_uart(int fd, int timeout, char *retString)
{
	int ret;
	int loop;
	const char *sendData = "12";
	const char *retData = "34";
	char recvBuf[64] = {0};

	if (!retString) {
		return -1;
	}

	ret = set_opt(fd, CONFIG_BAUDRATE, CONFIG_DATABIT,
				CONFIG_EVENT, CONFIG_STOPBIT);
	if (ret < 0) {
		LOGE("set_opt failed.\n");
		sprintf(retString, "NG:config_failed");
		return -2;
	}

	for (loop = 0; loop < timeout; loop++) {
		ret = write(fd, sendData, strlen(sendData));
		if (ret < 0 || ret != (int)strlen(sendData)) {
			LOGE("send data failed.\n");
			sprintf(retString, "NG:send_data_%d", ret);
			return -2;
		}

		ret = read(fd, recvBuf, sizeof(recvBuf));
		if (ret < 0) {
			LOGE("read data failed.\n");
			sprintf(retString, "NG:read_data_%d", ret);
			return -2;
		}

		if (ret > 0) {
			if (ret != (int)strlen(retData) ||
					strncmp(recvBuf, retData, strlen(retData))) {
				LOGE("receive invalid data.\n");
				LOGE("receive invalid data [%s] %d.\n", recvBuf, ret);
				sprintf(retString, "NG:ret_data_error");
				return -2;
			}

			sprintf(retString, "OK");
			return 0;
		}

		usleep(500000);
	}

	LOGE("wait for response data timeout.\n");
	sprintf(retString, "NG:TimeOut");
	return -1;
}

static int8_t gStopAte = 0;

void *do_ate(void *param)
{
	char *devPath = (char *)param;
	gStopAte = 0;

	int fd = open(devPath, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		LOGE("ate: open %s failed with %d.\n", devPath, fd);
		return NULL;
	}

	int ret = set_opt(fd, CONFIG_BAUDRATE, CONFIG_DATABIT,
				CONFIG_EVENT, CONFIG_STOPBIT);
	if (ret < 0) {
		LOGE("ate: set_opt failed.\n");
		return NULL;
	}

	const char *rxData = "12";
	const char *txData = "34";
	char buf[16] = {0};
	while (!gStopAte) {
		ret = read(fd, buf, sizeof(buf));
		if (ret > 0) {
			ret = strncmp(buf, rxData, strlen(rxData));
			if (ret != 0) {
				LOGE("ate: receive invalid data [%s], expect [%s]\n", buf, rxData);
				continue;
			}

			ret = write(fd, txData, strlen(txData));
		}
		usleep(100000);
	}

	close(fd);
	gStopAte = 0;
}

void *do_uart_test(void *param)
{
	int ret;
	int timeout = 5;
	int fd = -1;
        int nread;
	int loop;
	char recv_buf[32] = {0};
	struct packet_info info;

	char itemName[32] = {0};
	char devName[32] = {0};
	char devPath[64] = {0};
	char testType[16] = {0};
	char retString[32] = {0};

	LOGV("%s\n", __func__);
	memcpy(&info, param, sizeof(struct packet_info));

	if (info.unit_id < 0) {
		LOGE("invalid uart port%d\n", info.unit_id);
		sprintf(retString, "NG:Invalid_port");
		send_response_packet(&info, retString, strlen(retString));
		return NULL;
	}
		
	timeout = info.timeout;
	LOGV("timeout value %d\n", timeout);

	sprintf(itemName, "com%d", info.unit_id);
	ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
			itemName, "device", devName);
	if (ret != 0) {
		LOGW("attribute device not found, assume ttyS%d\n", info.unit_id);
		sprintf(devName, "ttyS%d", info.unit_id);
	}

	sprintf(devPath, "/dev/%s", devName);
	fd = open(devPath, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		LOGE("open %s failed with %d.\n", devPath, fd);
		sprintf(retString, "NG:open_failed");
		send_response_packet(&info, retString, strlen(retString));
		return NULL;
	}

	ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
			itemName, "type", testType);
	if (ret != 0) {
		LOGW("type not exist, assume normal uart\n");
		sprintf(testType, "uart");
	}

	if (strcmp(testType, "tsinput") == 0) {
		ret = test_tsinput(fd, timeout, retString);
	} else if (strcmp(testType, "selftest") == 0) {
		ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
				itemName, "ate", devName);
		if (ret != 0) {
			LOGE("ate not found on xml for selftest\n");
			sprintf(retString, "NG:no_ate");
			goto out;
		}

		sprintf(devPath, "/dev/%s", devName);

		pthread_create(&gAtePid, NULL, do_ate, devPath);

		ret = test_uart(fd, timeout, retString);

		gStopAte = 1;
		if (gAtePid) {
			if (pthread_join(gAtePid, NULL)) {
				LOGE("error join ate thread.");
				abort();
			}

			LOGD("exit\n");
			gAtePid = 0;
		}
	} else {
		ret = test_uart(fd, timeout, retString);
	}

out:
        close(fd);
	send_response_packet(&info, retString, strlen(retString));

	return NULL;
}
