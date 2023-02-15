#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config_parser.h"
//#include "cutils/properties.h"
#include "packet.h"
#include "test_item.h"

#define SCRIPT_TEST_FLASH "/usr/local/bin/test_storage.sh"

void *do_flash_test(void *param)
{
	int ret;
	char ret_string[32] = {0};
	char devNode[128] = {0};
	char cmd[256] = {0};
	struct packet_info info;

	LOGV("%s\n", __func__);
	memcpy(&info, param, sizeof(struct packet_info));

	do {
		ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM, "flash", "device", devNode);
		if (ret != 0) {
			LOGE("get device node for flash failed.\n");
			sprintf(ret_string, "NG:dev_not_labeled");
			break;
		}

		sprintf(cmd, "%s %s vfat /tmp/storage true", SCRIPT_TEST_FLASH, devNode);
		ret = __system(cmd);
		if (ret) {
			LOGE("test flash failed with %d.\n", ret);
			sprintf(ret_string, "NG:test_failed");
			break;
		}

		sprintf(ret_string, "OK");
	} while(0);

	send_response_packet(&info, ret_string, strlen(ret_string));

	return NULL;
}
