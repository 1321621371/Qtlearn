#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "config_parser.h"
#include "packet.h"
#include "test_item.h"

#define SCRIPT_TEST_USB	"/usr/local/bin/test_storage.sh"

int getDiskFsType(char* devName, char *diskType);

void *do_usb_test(void *param)
{
	int ret;
	char ret_string[32] = {0};
	char devName[32] = {0};
	char sysNode[128] = {0};
	char devNode[64] = {0};
	char cmd[128] = {0};
	struct packet_info info;
	char diskFsType[10];

	LOGV("%s\n", __func__);
	memcpy(&info, param, sizeof(struct packet_info));

	do {
		if (info.unit_id < 0) {
			LOGE("invalid usb port: %d\n", info.unit_id);
			sprintf(ret_string, "NG:Invalid_port");
			break;
		}

		sprintf(devName, "usb%d", info.unit_id);
		ret = config_get_attr(SECTION_DEVICES, SECTION_DEVICES_DEVICE,
				devName, SECTION_DEVICES_DEVICE_SYSFS, sysNode);
		if (ret != 0) {
			LOGE("get sysfs node for %s failed.\n", devName);
			sprintf(ret_string, "NG:dev_not_labeled");
			break;
		}

		//TODO: disable USB3.0 controller

		ret = find_dev_node(sysNode, "sd", devNode);
		if (ret != 0) {
			LOGE("get dev node for %s failed.\n", devName);
			sprintf(ret_string, "NG:dev_not_connected");
			break;
		}

		LOGD("%s dev node: %s\n", devName, devNode);
		if(getDiskFsType(devNode, diskFsType) < 0)
		{
			sprintf(ret_string, "NG:file_system_not_surpported");
			send_response_packet(&info, ret_string, strlen(ret_string));
			return NULL;

		}

		sprintf(cmd, "%s %s %s /tmp/storage true", SCRIPT_TEST_USB, devNode, diskFsType);
		LOGI("system:%s\n", cmd);
		ret = __system(cmd);
		if (ret) {
			LOGE("test %s failed with %d.\n", devName, ret);
			sprintf(ret_string, "NG:test_failed");
			break;
		}

		sprintf(ret_string, "OK");
	} while(0);

	send_response_packet(&info, ret_string, strlen(ret_string));
	return NULL;
}
