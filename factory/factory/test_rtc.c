#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define __USE_XOPEN
#include <time.h>

#include "packet.h"
#include "test_item.h"
#include "common.h"

#define DEV_RTC0 "/dev/rtc0"

static int read_rtc_time(char *buffer, int buffer_size)
{
	int ret;
	time_t time_local;
	struct rtc_time tm_tmp;
	struct rtc_time tm_local;
	int fd;
	char param[MAX_DATA_SIZE];

	memset(&param, 0, MAX_DATA_SIZE);
	fd = open(DEV_RTC0, O_RDONLY);
	if (fd < 0) {
		printf("%s open fail\n", DEV_RTC0);
		ret = -1;
	} else {
		ret = ioctl(fd, RTC_RD_TIME, (unsigned long)&tm_tmp);
		printf("tm_tmp year=%d, mon=%d, day=%d, hour=%d, min=%d, sec=%d\n", \
		tm_tmp.tm_year, tm_tmp.tm_mon, tm_tmp.tm_mday, tm_tmp.tm_hour, tm_tmp.tm_min, tm_tmp.tm_sec);
		time_local = timelocal((struct tm*)&tm_tmp) + 8*60*60;
		localtime_r(&time_local, (struct tm*)&tm_local);
		if (ret < 0) {
			printf("%s read fail\n", DEV_RTC0);
			ret = -1;
		} else {
			strftime((char*)param, MAX_DATA_SIZE, "%Y.%m.%d-%H.%M.%S", (struct tm*)&tm_local);
			printf("tm_local year=%d, mon=%d, day=%d, hour=%d, min=%d, sec=%d\n", \
					tm_local.tm_year, tm_local.tm_mon, tm_local.tm_mday, \
					tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);
			ret = 0;
		}
		close(fd);
	}
	memcpy(buffer, &param, buffer_size);

	return ret;
}

int write_rtc_time(char *buffer, int buffer_size)
{
	int ret;
	time_t time_utc;
	struct rtc_time tm_tmp;
	//struct rtc_time tm_utc;
	int fd;
	char param[MAX_DATA_SIZE];

	memset(&param, 0, MAX_DATA_SIZE);
	fd = open(DEV_RTC0, O_RDWR);
	if(fd < 0) {
		printf("%s open fail\n", DEV_RTC0);
		ret = -1;
	} else {
		memcpy(&param, buffer, buffer_size);
		strptime((char*)param, "%Y.%m.%d-%H.%M.%S", (struct tm*)&tm_tmp);
		printf("tm_tmp year=%d, mon=%d, day=%d, hour=%d, min=%d, sec=%d\n", \
				tm_tmp.tm_year, tm_tmp.tm_mon, tm_tmp.tm_mday, \
				tm_tmp.tm_hour, tm_tmp.tm_min, tm_tmp.tm_sec);
		//time_utc = timelocal((struct tm*)&tm_tmp) - 8*60*60;
		//localtime_r(&time_utc,(struct tm*)&tm_utc);
		//printf("tm_rtc year=%d, mon=%d, day=%d, hour=%d, min=%d, sec=%d\n", \
				tm_utc.tm_year, tm_utc.tm_mon, tm_utc.tm_mday, \
				tm_utc.tm_hour, tm_utc.tm_min, tm_utc.tm_sec);
		//ret = ioctl(fd, RTC_SET_TIME, (unsigned long)&tm_utc);
		ret = ioctl(fd, RTC_SET_TIME, (unsigned long)&tm_tmp);
		if (ret < 0) {
			printf("%s write fail\n", DEV_RTC0);
			ret = -2;
		}
		close(fd);
	}
	return ret;
}

void *do_rtc_test(void *param)
{
	struct packet_info info;
	char stb_timer[32] = {0};
	char ret_string[32] = {0};
	int ret;

	printf("%s\n", __func__);

	memcpy(&info, param, sizeof(struct packet_info));

	if (info.type == ITEM_TYPE_GET) {
		//get rtc time
		printf("get rtc time...\n");
		ret = read_rtc_time(stb_timer, sizeof(stb_timer));
		if (ret < 0) {
			sprintf(ret_string, "1970.01.01-00.00.00");
		}
		else {
			snprintf(ret_string, sizeof(ret_string), "%s", stb_timer);
		}
	} else if (info.type == ITEM_TYPE_SET) {
		//set SN
		printf("set rtc time...\n");
		char err[128] = {0};
		ret = write_rtc_time(info.param, strlen(info.param));
		if (ret == -1) {
			sprintf(err, "rtc-open-fail");
		} else if (ret == -2) {
			sprintf(err, "rtc-write-fail");
		}

		if (ret < 0) {
			sprintf(ret_string, "NG;%s", err);
			printf("%s write err:%s\n", DEV_RTC0, err);
			goto out;
		}

		printf("write time:%s len:%d\n", info.param, strlen(info.param));

		//sync to system clock
		char cmd[64];
		sprintf(cmd, "hwclock -s -u");
		__system(cmd);

		ret = read_rtc_time(stb_timer, sizeof(stb_timer));
		if (ret < 0) {
			sprintf(ret_string, "NG;1970.01.01-00.00.00");
			goto out;
		}

		snprintf(ret_string, sizeof(ret_string), "OK;%s", stb_timer);
	}

out:
	send_response_packet(&info, ret_string, strlen(ret_string));

	return (void *)NULL;
}
