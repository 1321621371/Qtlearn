#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "config_parser.h"
#include "network.h"
#include "packet.h"
#include "test_item.h"

static int ensure_link_up(const char *itemName, const char *ethName, unsigned int timeout)
{
	int ret;
	char ipAddr[128] = {0};
	char cmd[64] = {0};

	if (!itemName || !ethName)
		return -1;

	ret = get_eth_link_state(ethName);
	if (ret < 0) {
		LOGE("get linkstate for %s failed.\n", ethName);
		return -2;
	}

	if (ret == ETH_LINK_UP) {
		return 0;
	}

	LOGD("try setup ip\n");
	ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
				itemName, "ip", ipAddr);
	if (ret != 0) {
		LOGE("get ip config for %s failed.\n", itemName);
		return -3;
	}

	LOGD("%s ipaddr: %s\n", ethName, ipAddr);
	sprintf(cmd, "ifconfig %s %s", ethName, ipAddr);
	ret =  __system(cmd);
	if (ret != 0) {
		LOGE("set ip failed %d\n", ret);
		return -4;
	}

	//TODO: NETLINK
	while(timeout-- > 0) {
		ret = get_eth_link_state(ethName);
		if (ret == ETH_LINK_UP) {
			return 0;
		}

		sleep(1);
	}

	return -5;
}

static int check_link_speed(const char *itemName, const char *ethName, char *ret_string)
{
	int ret;
	char configLinkSpeed[32] = {0};
	int linkSpeed = 0;

	if (!itemName || !ethName)
		return -1;

	ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
			itemName, "speed", configLinkSpeed);
	if (ret != 0) {
		LOGW("get speed config for %s failed.\n", itemName);
		return 0;
	}

	linkSpeed = atoi(configLinkSpeed);

	ret = get_eth_link_speed(ethName);
	if (ret < 0) {
		LOGE("get eth link speed failed.\n");
		if (ret_string != NULL)
			sprintf(ret_string, "NG:get_link_speed");
		return -1;
	}

	if (ret != linkSpeed) {
		//TODO: negotiation
		LOGE("link speed mismatch: (%d!=%d).\n", ret, linkSpeed);
		if (ret_string != NULL)
			sprintf(ret_string, "NG:link_speed_mismatch");
		return -1;
	}

	return 0;
}


void *do_net_test(void *param)
{
	int ret;
	char ret_string[32] = {0};
	char cmd[64] = {0};
	int timeout;
	struct packet_info info;
	char itemName[32] = {0};
	char ethName[32] = {0};
	char serverIpAddr[128] = {0};
	char linkSpeed[32] = {0};
	int net_num = -1;

	LOGV("%s\n", __func__);
	memcpy(&info, param, sizeof(struct packet_info));

	do {
                if (info.unit_id < 0) {
                        LOGE("invalid net port%d\n", info.unit_id);
                        sprintf(ret_string, "NG:Invalid_port");
                        break;
                }

		timeout = info.timeout;
		if (timeout <= 0) {
			timeout = 5;
		}

		//parse target ip
#if 0
		ret = parse_param_value(info.param, info.param_size,
					"serverIp", serverIpAddr);
                if (ret < 0) {
                        LOGE("parse serverIp failed.\n");
                        sprintf(ret_string, "NG:Invalid_serverIp");
                        break;
                }
		LOGD("serverIp: %s\n", serverIpAddr);
#endif

		//parse device name
		//if (info.unit_id != 1){
		 //   LOGD("unit error, fail get eth%d", info.unit_id);
		//}else{
		   net_num = info.unit_id; 
		//}
		sprintf(itemName, "eth%d", net_num);
		ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
				itemName, "device", ethName);
		if (ret != 0) {
			LOGE("get device name for %s failed.\n", itemName);
			sprintf(ret_string, "NG;dev_not_labeled");
			break;
		}


		ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
				itemName, "targetIp", serverIpAddr);
		if (ret != 0) {
			LOGE("get targetIp for %s failed.\n", itemName);
			sprintf(ret_string, "NG;targetIp_not_labeled");
			break;
		}
		LOGD("serverIp: %s\n", serverIpAddr);

		//setup ip, wait link up
		ret = ensure_link_up(itemName, ethName, timeout - 1);
		if (ret != 0) {
			LOGE("wait link up failed with %d\n", ret);
			sprintf(ret_string, "NG;link_state");
			break;
		}

		//check link speed
#if 0
		ret = check_link_speed(itemName, ethName, ret_string);
		if (ret != 0) {
			LOGE("check linkspeed failed.\n");
			break;
		}
#endif

		//do ping test
		sprintf(cmd, "ping -I %s %s -w %d -c 1", ethName, serverIpAddr, (timeout - 1));
		ret =  __system(cmd);
		if (ret) {
			LOGE("ping failed with %d\n", ret);
			sprintf(ret_string, "NG:ping_failed");
			break;
		}

		sprintf(ret_string, "OK");
	} while(0);

	send_response_packet(&info, ret_string, strlen(ret_string));
	return NULL;
}
