#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <termios.h>

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "common.h"
#include "packet.h"
#include "test_item.h"
#include "config_parser.h"

#define MAX_AT_RESPONSE 1024
static char s_ATBufferExtern[MAX_AT_RESPONSE+1];
static char *s_ATBufferExternCur = s_ATBufferExtern;
static int s_readCount = 0;
static int is_sim_test_done = 0;
int isAtResponseExtern = 0;
int isResponseSucExtern = 0;
int isAtSendedExtern = 0;
char gAtAck_command_extern[20]={"\0"};
char returnResult[30]={"\0"};

static int gStopRx;
static pthread_mutex_t gAtMutex = PTHREAD_MUTEX_INITIALIZER;


static char *findNextEOLExtern(char *cur)
{
	if (cur[0] == '>' && cur[1] == ' ' && cur[2] == '\0') {
		/* SMS prompt character...not \r terminated */
		return cur + 2;
	}

	//Find next newline
	while (*cur != '\0' && *cur != '\r' && *cur != '\n')
		cur++;

	return *cur == '\0' ? NULL : cur;
}

static const char *readLine(int fd)
{
	int count;

	char *p_read = NULL;
	char *p_eol = NULL;
	char *ret;

	if (*s_ATBufferExternCur == '\0') {
		s_ATBufferExternCur = s_ATBufferExtern;
		*s_ATBufferExternCur = '\0';
		p_read = s_ATBufferExtern;
	} else {
        while (*s_ATBufferExternCur == '\r' || *s_ATBufferExternCur == '\n')
            s_ATBufferExternCur++;

        p_eol = findNextEOLExtern(s_ATBufferExternCur);

        if (p_eol == NULL) {
            size_t len;

            len = strlen(s_ATBufferExternCur);

            memmove(s_ATBufferExtern, s_ATBufferExternCur, len + 1);
            p_read = s_ATBufferExtern + len;
            s_ATBufferExternCur = s_ATBufferExtern;
        }
    }

    while (p_eol == NULL) {
        if (0 == MAX_AT_RESPONSE - (p_read - s_ATBufferExtern)) {
            //LOGE("ERROR: Input line exceeded buffer\n");
            s_ATBufferExternCur = s_ATBufferExtern;
            *s_ATBufferExternCur = '\0';
            p_read = s_ATBufferExtern;
        }

        do {
		//LOGD("read..\n");
                //ui_print_xy_rgba(0,get_cur_print_y(),0,0,255,100,"start to read\n");
            count = read(fd, p_read,
                            MAX_AT_RESPONSE - (p_read - s_ATBufferExtern));
		//LOGD("read return %d\n", count);
        } while (count < 0 && errno == EINTR);

        if (count > 0) {
            //AT_DUMP( "<< ", p_read, count );
            s_readCount += count;
//ui_print_xy_rgba(0,get_cur_print_y(),0,0,255,100,"count:%d\n",count);
            p_read[count] = '\0';

            while (*s_ATBufferExternCur == '\r' || *s_ATBufferExternCur == '\n')
                s_ATBufferExternCur++;

            p_eol = findNextEOLExtern(s_ATBufferExternCur);
            p_read += count;
        } else if (count <= 0) {
            if(count == 0) {
                //LOGD("atchannel: EOF reached");
            } else {
                //LOGD("atchannel: read error %s", strerror(errno));
            }
            return NULL;
        }
    }


    ret = s_ATBufferExternCur;
    *p_eol = '\0';
    s_ATBufferExternCur = p_eol + 1;

    //LOGD("AT< %s\n", ret);
    return ret;
}

static int gRxExit = 1;

static void *do_at_read(void *arg)
{
	const char *line;
	int count;
        char sread;
        int cur=0;

	int fd = *(int*)arg;

	gRxExit = 0;

	while (!gStopRx) {
		line = readLine(fd);

		pthread_mutex_lock(&gAtMutex);

		if (isAtSendedExtern) {
                        //LOG("%s line=%d  readline=%s, gAtAck_command_extern=%s\n", __FUNCTION__, __LINE__, line, gAtAck_command_extern);

			if (line && strncmp(gAtAck_command_extern, "ATE", 3) == 0) {
                        	sprintf(returnResult,"%s",line);
                                isResponseSucExtern = 1;
                                isAtResponseExtern = 1;
                                isAtSendedExtern =0;
                		pthread_mutex_unlock(&gAtMutex);
				continue;
			}

                        if(line != NULL){
                                if(line != NULL && strstr(line, gAtAck_command_extern)!=NULL) {
                                        line = readLine(fd);
                                        //LOG("%s line=%d readline=%s  \n", __FUNCTION__, __LINE__, line);
                                        if(line != NULL && strstr(line,"ERROR") == NULL){
                                                sprintf(returnResult,"%s",line);
                                                isResponseSucExtern = 1;
                                                isAtResponseExtern = 1;
                                                isAtSendedExtern =0;
                                        } else if(line != NULL && strstr(line,"ERROR") != NULL) {
                                                isResponseSucExtern = 0;
                                                isAtResponseExtern = 1;
                                                isAtSendedExtern =0;
                                        }
                                }

                        }
                }

                pthread_mutex_unlock(&gAtMutex);

		usleep(100000);
        }

	LOGD("rx exit\n");
	gRxExit = 1;

        return NULL;
}

int at_send_extern(int fd,char *send_command)
{
        int cur = 0;
        int len = strlen(send_command);
        int written;
        int i = 3;
        int j=0;
        for(j=0;j<3;j++){

                pthread_mutex_lock(&gAtMutex);

                isResponseSucExtern = 0;
                isAtResponseExtern = 0;
                isAtSendedExtern =0;
                cur = 0;
                while (cur < len) {
                        written = write (fd, send_command + cur, len - cur);
                        if (written < 0) {
                                pthread_mutex_unlock(&gAtMutex);
                                return -1;
                        }
                        cur += written;
                }

                isAtSendedExtern = 1;

                pthread_mutex_unlock(&gAtMutex);

                i = 6;
          while(1){
                usleep(500000);
                if(isAtResponseExtern && isResponseSucExtern ){
                        isResponseSucExtern = 0;
                        isAtResponseExtern = 0;
                        isAtSendedExtern =0;
                        return 1;
                }else  if(isAtResponseExtern && isResponseSucExtern==0){
                        isResponseSucExtern = 0;
                        isAtResponseExtern = 0;
                        isAtSendedExtern =0;
                        break;
                }

                i--;
                if(i<=0){
                        isResponseSucExtern = 0;
                        isAtResponseExtern = 0;
                        isAtSendedExtern =0;
                        break;
                }
          }
        }
        return -1;
}

void* do_4g_test(void *param)
{
	int ret;
	char retString[64] = {0};
	struct packet_info info;
	char itemName[32] = {0};
	char devName[32] = {0};
	char devPath[32] = {0};

	memcpy(&info, param, sizeof(struct packet_info));

	sprintf(itemName, "4g%d", info.unit_id);
	ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
			itemName, "device", devName);
	if (ret != 0) {
		LOGE("config error, %s:%s:%s:device not found\n", SECTION_PCBA, SECTION_PCBA_ITEM, itemName);
		sprintf(retString, "NG:config_error");
		send_response_packet(&info, retString, strlen(retString));
                return NULL;
	}
	sprintf(devPath, "/dev/%s", devName);

	int fd = open(devPath,O_RDWR);
	if (fd < 0) {
		LOGE("Failed to open serial port\n");
		sprintf(retString, "NG;open_dev");
                send_response_packet(&info, retString, strlen(retString));
		return NULL;
	}

        char cmd[64] = {0};
        sprintf(cmd, "ps -aux | grep pppd | head -1 | awk '{print $2}' | xargs kill -9");
        system(cmd);

        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	//setup serial port
	struct termios ios;
	tcgetattr(fd, &ios);
	ios.c_lflag = 0;
	cfsetispeed(&ios, B115200);
	cfsetospeed(&ios, B115200);
	cfmakeraw(&ios);
	tcsetattr(fd, TCSANOW, &ios);

	//create at read thread
	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	gStopRx = 0;

	pthread_t tid;
	ret = pthread_create(&tid, &attr, do_at_read, &fd);
	//ret = pthread_create(&tid, NULL, do_at_read, &fd);
	if (ret < 0) {
		LOGE("Failed to create pthread\n");
		close(fd);
		sprintf(retString, "NG;create_pthread");
                send_response_packet(&info, retString, strlen(retString));
		return NULL;
	}

	//enable echo
	LOGD("enable echo\n");
	char *expectAck = "ATE";
        strncpy(gAtAck_command_extern, expectAck, strlen(expectAck));
        gAtAck_command_extern[strlen(expectAck)] = '\0';
        if (at_send_extern(fd,"ATE1\r\n") < 0) {
                LOGE("execute ATE fail\n");
		sprintf(retString, "NG;enable_echo");
		goto out;
        }

	//check sim status
	LOGD("check sim status\n");
	expectAck = "AT+CPIN?";
        strncpy(gAtAck_command_extern, expectAck, strlen(expectAck));
        gAtAck_command_extern[strlen(expectAck)] = '\0';
        if (at_send_extern(fd,"AT+CPIN?\r\n") < 0) {
                LOGE("execute AT+CPIN? fail\n");
		sprintf(retString, "NG;get_sim_status");
		goto out;
        }

	if (!strstr(returnResult, "READY")) {
                LOGE("sim not ready\n");
		sprintf(retString, "NG;sim_not_ready");
		goto out;
	}

	//check signal
	LOGD("check signal\n");
	expectAck = "AT+CSQ";
        strncpy(gAtAck_command_extern, expectAck, strlen(expectAck));
        gAtAck_command_extern[strlen(expectAck)] = '\0';
        if (at_send_extern(fd,"AT+CSQ\r\n") < 0) {
                LOGE("execute AT+CSQ fail\n");
		sprintf(retString, "NG;get_signal");
		goto out;
        }

	char *sub = strtok(returnResult + 6, ",");
        while (sub != NULL) {
                LOGD("signal = [%s]\n", sub);

		int val = atoi(sub);
		if (val == 99 || val == 199 || val < 4) {
                	LOGE("terrible signal\n");
			sprintf(retString, "NG;signal<%d>", val);
			goto out;
		} else {
			sprintf(retString, "OK;signal<%d>", val);
			goto out;
		}
        }

out:
	gStopRx = 1;
	LOGD("wait rx\n");
	pthread_join(tid, NULL);

	close(fd);

	//LOGD("wait rx\n");
	//while (!gRxExit)
	//	;

	LOGD("return\n");
	send_response_packet(&info, retString, strlen(retString));

        return 0;
}
