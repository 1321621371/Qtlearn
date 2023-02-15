#include <stdio.h>

#define __USE_GNU
#include <string.h>

#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "common.h"
#include "packet.h"
#include "udp.h"
#include "test_item.h"

static struct packet_info g_packet_info;

static struct cmd_mapper mapper[] = {
	{"4G",		do_4g_test},
	{"BSN",		handle_bsn},
	{"CAN",		do_can_test},
	{"COM",		do_uart_test},
	{"FLASH",	do_flash_test},
	{"FsVer",	get_firmware_version},
	{"GPS",		do_gps_test},
	{"Mode",	handle_stb_mode},
	{"NET",		do_net_test},
	{"RTC",		do_rtc_test},
	{"USB",		do_usb_test},
};

static int parse_packet(char *packet, int pack_size)
{
	int ret = -1;
	char *sub = NULL;
	size_t tmp;
	char* contents[MAX_PACKET_CONTENT_LENGTH];
	size_t content_size = 0;
	char *head, *type, *item, *param, *tail;
	UNUSED(pack_size);

	int item_type;
	int item_id = -1;

	memset(contents, 0, sizeof(contents));

	sub = strtok(packet, DATA_SPLIT_TAG);	
	while (sub != NULL) {
		LOGV("packet%d = [%s]\n", content_size, sub);
		if (content_size < sizeof(contents)) {
			contents[content_size] = sub;
		}

		content_size++;

		sub = strtok(NULL, DATA_SPLIT_TAG);
	}

	if ((content_size < MIN_PACKET_CONTENT_LENGTH) ||
		(content_size > MAX_PACKET_CONTENT_LENGTH)) {
		LOGE("invalid packet length %d.\n", content_size);
		return ret;
	}

	head = contents[INDEX_PACKET_HEAD];	
	type = contents[INDEX_PACKET_TYPE];	
	item = contents[INDEX_PACKET_ITEM];
	param = contents[content_size - 2];
	tail = contents[content_size - 1];

	if (strncmp(head, PACKET_HEAD, strlen(PACKET_HEAD)) ||
			strncmp(tail, PACKET_TAIL, strlen(PACKET_TAIL))) {
		LOGE("invalid format with [%s] and [%s].\n", head, tail);
		return ret;
	}

	if (!strncmp(type, PACKET_TYPE_GET, strlen(PACKET_TYPE_GET))) {
		item_type = ITEM_TYPE_GET;
	} else if (!strncmp(type, PACKET_TYPE_SET, strlen(PACKET_TYPE_SET))) {
		item_type = ITEM_TYPE_SET;
	} else {
		LOGE("invalid type [%s].\n", type);
		return ret;
	}

	for (tmp = 0; tmp < sizeof(mapper) / sizeof(struct cmd_mapper); tmp++) {
		if (!strcmp(item, mapper[tmp].command)) {
			item_id = tmp;
			break;
		}
	}

	if (item_id == -1) {
		LOGE("item [%s] not supported.\n", item);
		return ret;
	}

	memset(&g_packet_info, 0, sizeof(struct packet_info));

	g_packet_info.type = item_type;
	g_packet_info.item_id = item_id;
	g_packet_info.unit_id = -1;

	if (content_size == MAX_PACKET_CONTENT_LENGTH) {
		g_packet_info.unit_id = atoi(contents[INDEX_PACKET_UNIT]);
	}

	//parse timeout value
	char timeout[8] = {0};
	if (parse_param_value(param, strlen(param), TIMEOUT_VAL_TAG, timeout) == 0)
		g_packet_info.timeout = atoi(timeout);
	else
		g_packet_info.timeout = 5;

	//remove timeout from param
	sub = strcasestr(param, TIMEOUT_VAL_TAG);
	if (sub != NULL) {
		if ((unsigned int)(sub) - (unsigned int)param > 0)
			*(sub - 1) = '\0';
		else
			*sub = '\0';
	}

	memset(g_packet_info.param, 0, MAX_DATA_SIZE);
	memcpy(g_packet_info.param, param, strlen(param));
	g_packet_info.param_size = strlen(param);

	LOGV("type=%d, unit=%d, item=%d, timeout=%d\n",
		g_packet_info.type,
		g_packet_info.unit_id,
		g_packet_info.item_id,
		g_packet_info.timeout);
	LOGV("param=[%s], size=%d\n", g_packet_info.param, g_packet_info.param_size);

	return 0;
}

static int getItermID(char *iterm_name)
{
	unsigned int i, item_id=0;
	if(iterm_name == NULL){
		return -1;
	}
	for (i = 0; i < sizeof(mapper) / sizeof(struct cmd_mapper); i++) {
		if (!strcmp(iterm_name, mapper[i].command)) {
			item_id = i;
			break;
		}
	}
	return item_id;
}

static pthread_t mPid = 0;

void handle_packet(char *packet, int size)
{
	struct packet_info *info;
	//char ip[PROPERTY_VALUE_MAX] = {0};
	char ip[32] = {0};
	int item_id=0;
	if (parse_packet(packet, size) != 0) {
		return;
	}

	info = &g_packet_info;

	LOGD("command: %s\n", mapper[info->item_id].command);

	//TODO: 停止正在运行的测试线程 开始新的测试线程;
	if (mPid) {
		pthread_kill(mPid, SIGALRM);
		if (pthread_join(mPid, NULL)) {
			LOGE("error join thread.");
			abort();
		}
	}
	//property_get(PROP_SLAVE_IP, ip, "N/A");
//ycn TODO	: ip
#if 0
	if (!strcmp("Mode", mapper[info->item_id].command)&&strcmp(ip,"N/A")) {
		item_id = getItermID("MsgRouter");
		pthread_create(&mPid, NULL, mapper[item_id].handler, info);
		display_test_pattern();
	}else{
		//get mode \set mode need to msg router	
		if(info->unit_id > SLAVE_MAGIC_NUM && strcmp(ip,"N/A")){
			item_id = getItermID("MsgRouter");
			pthread_create(&mPid, NULL, mapper[item_id].handler, info);
		}else if(info->unit_id < SLAVE_MAGIC_NUM){
			pthread_create(&mPid, NULL, mapper[info->item_id].handler, info);
		}
	}
#else
	pthread_create(&mPid, NULL, mapper[info->item_id].handler, info);
#endif
}


int parse_param_value(char *in, int size, char *key, char *ret_string)
{
	char *sub;
	char *tmp;
	char buf[MAX_DATA_SIZE] = {0};

	if (!in || size == 0 || !key || !ret_string) {
		return -1;	
	}

	memcpy(buf, in, size);

	sub = strcasestr(buf, key);
	if (!sub) {
		return -1;
	}

	//key:value;
	tmp = strtok(sub + strlen(key) + 1, PARAM_SPLIT_TAG);
	if (!tmp) {
		tmp = sub + strlen(key) + 1;
	}

	memcpy(ret_string, tmp, strlen(tmp));

	return 0;
}


int send_response_packet(struct packet_info *info, char *message, int size)
{
	char packet[MAX_DATA_SIZE];
	int packet_size = 0;

	LOGV("%s\n", __func__);

	memset(packet, 0, sizeof(packet));

	if (size > (MAX_DATA_SIZE - 12)) {
		LOGE("[Error] too much data.\n");
		return -1;
	}

	if (info->item_id != g_packet_info.item_id) {
		LOGW("test item changed [%d --> %d], ignore.\n",
			info->item_id, g_packet_info.item_id);
		return 0;
	}

	if (info->unit_id != -1) {
		sprintf(packet, "%s %s %s %d ", PACKET_HEAD, PACKET_TYPE_RET,
				mapper[info->item_id].command, info->unit_id);
	} else {
		sprintf(packet, "%s %s %s ", PACKET_HEAD, PACKET_TYPE_RET,
					mapper[info->item_id].command);

	}

	packet_size += strlen(packet);

	strcat(packet, message);
	packet_size += size;

	strcat(packet, DATA_SPLIT_TAG);
	packet_size += strlen(DATA_SPLIT_TAG);

	strcat(packet, PACKET_TAIL);
	packet_size += strlen(PACKET_TAIL);

	packet[packet_size] = '\0';

	LOGD("send data [%s]\n", packet);

	return send_udp_data(packet, packet_size);
}

int repack_content(char *packet,struct packet_info *info){

	int packet_size = 0;
	LOGV("%s\n", __func__);

	if (info->unit_id != -1) {
		sprintf(packet, "%s %s %s %d ", PACKET_HEAD, info->type ? PACKET_TYPE_SET:PACKET_TYPE_GET,
				mapper[info->item_id].command, info->unit_id);
	}else {
		sprintf(packet, "%s %s %s ", PACKET_HEAD, info->type ? PACKET_TYPE_SET:PACKET_TYPE_GET,
					mapper[info->item_id].command);

	}
	packet_size += strlen(packet);

	char tmp[20]={0};
	memset(tmp,0,sizeof(tmp));
	if(info->param_size ==0){
		sprintf(tmp,"TimeOut:%d",info->timeout);
		strcat(packet,tmp);
		packet_size +=strlen(tmp);
	}else{
		strcat(packet,info->param);
		packet_size += info->param_size;
		sprintf(tmp,";TimeOut:%d",info->timeout);
		strcat(packet,tmp);
		packet_size +=strlen(tmp);
	}

	strcat(packet, DATA_SPLIT_TAG);
	packet_size += strlen(DATA_SPLIT_TAG);

	strcat(packet, PACKET_TAIL);
	packet_size += strlen(PACKET_TAIL);

	packet[packet_size] = '\0';

	LOGD("repack data: %s\n", packet);
	return packet_size;

}

int repack_ret_content(char *packet,struct packet_info *info){
	if (info->unit_id != -1) {
		sprintf(packet, "%s %s %s %d %s #", PACKET_HEAD, PACKET_TYPE_RET,
				mapper[info->item_id].command, info->unit_id,info->param);
	}else {
		sprintf(packet, "%s %s %s %s #", PACKET_HEAD, PACKET_TYPE_RET,
					mapper[info->item_id].command,info->param);
	}
	return strlen(packet);
}

int parse_ret_packet(char *packet, int pack_size,struct packet_info *info)
{
	int ret = -1;
	char *sub = NULL;
	size_t tmp;
	char* contents[MAX_PACKET_CONTENT_LENGTH];
	size_t content_size = 0;
	char *head, *type, *item, *param, *tail;

	int item_type;
	int item_id = -1;

	UNUSED(pack_size);
	memset(contents, 0, sizeof(contents));

	sub = strtok(packet, DATA_SPLIT_TAG);	
	while (sub != NULL) {
		LOGV("packet%d = [%s]\n", content_size, sub);
		if (content_size < sizeof(contents)) {
			contents[content_size] = sub;
		}
		content_size++;
		sub = strtok(NULL, DATA_SPLIT_TAG);
	}

	if ((content_size < MIN_PACKET_CONTENT_LENGTH) ||
		(content_size > MAX_PACKET_CONTENT_LENGTH)) {
		LOGE("invalid packet length %d.\n", content_size);
		return ret;
	}

	head = contents[INDEX_PACKET_HEAD];	
	type = contents[INDEX_PACKET_TYPE];	
	item = contents[INDEX_PACKET_ITEM];
	param = contents[content_size - 2];
	tail = contents[content_size - 1];

	if (strncmp(head, PACKET_HEAD, strlen(PACKET_HEAD)) ||
	      strncmp(tail, PACKET_TAIL, strlen(PACKET_TAIL))) {
		LOGE("invalid format with [%s] and [%s].\n", head, tail);
		return ret;
	}

	if (!strncmp(type, PACKET_TYPE_GET, strlen(PACKET_TYPE_GET))) {
		item_type = ITEM_TYPE_GET;
	} else if (!strncmp(type, PACKET_TYPE_SET, strlen(PACKET_TYPE_SET))) {
		item_type = ITEM_TYPE_SET;
	} else if (!strncmp(type, PACKET_TYPE_RET, strlen(PACKET_TYPE_RET))) {
		item_type = ITEM_TYPE_RET;
	}else {
		LOGE("invalid type [%s].\n", type);
		return ret;
	}

	for (tmp = 0; tmp < sizeof(mapper) / sizeof(struct cmd_mapper); tmp++) {
		if (!strcmp(item, mapper[tmp].command)) {
			item_id = tmp;
			break;
		}
	}

	if (item_id == -1) {
		LOGE("item [%s] not supported.\n", item);
		return ret;
	}

	memset(info, 0, sizeof(struct packet_info));

	info->type = item_type;
	info->item_id = item_id;
	info->unit_id = -1;

	if (content_size == MAX_PACKET_CONTENT_LENGTH) {
		info->unit_id = atoi(contents[INDEX_PACKET_UNIT]);
	}

	memset(info->param, 0, MAX_DATA_SIZE);
	memcpy(info->param, param, strlen(param));
	info->param_size = strlen(param);

	return 0;
}
