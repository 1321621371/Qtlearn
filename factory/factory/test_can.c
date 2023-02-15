#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "common.h"
#include "config_parser.h"
#include "packet.h"
#include "test_item.h"

#define SCRIPT_TEST_CAN	"/usr/local/bin/test_can.sh"

void *do_can_test(void *param)
{
	int ret;
	char ret_string[32] = {0};
	char itemName[32] = {0};
	char testType[32] = {0};
	char devName[32] = {0};
	char ateName[32] = {0};
	char cmd[128] = {0};
	pid_t status;
	struct packet_info info;

	LOGV("%s\n", __func__);
	memcpy(&info, param, sizeof(struct packet_info));

	do {
		if (info.unit_id < 0) {
			LOGE("invalid can id: %d\n", info.unit_id);
			sprintf(ret_string, "NG:Invalid_id");
			break;
		}

		sprintf(itemName, "can%d", info.unit_id);

		ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
				itemName, "type", testType);
		if (ret != 0) {
			LOGW("type not exist, assume selftest\n");
			sprintf(testType, "selftest");
		}

		if (strcmp(testType, "selftest") != 0) {
			LOGW("unknown testType: %s\n", testType);
			sprintf(ret_string, "NG:can_test_type");
			break;
		}

		ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
				itemName, "device", devName);
		if (ret != 0) {
			LOGW("can device not exist in xml\n");
			sprintf(ret_string, "NG:can_device");
			break;
		}

		ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
				itemName, "ate", ateName);
		if (ret != 0) {
			LOGW("can ate not exist in xml\n");
			sprintf(ret_string, "NG:can_ate");
			break;
		}

		sprintf(cmd, "%s %s %s", SCRIPT_TEST_CAN, devName, ateName);
		ret = __system(cmd);
		printf("wlxwlxexit status value = [0x%x]\n", status);
    		if (WIFEXITED(status))
    		{
      			if (WEXITSTATUS(status))
      			{
				printf("run fail, exit code: %d\n", WEXITSTATUS(status));
				LOGE("test %s failed with %d.\n", devName, ret);
				sprintf(ret_string, "NG:test_failed");
				 printf("wlxwlxrun shell script fail, script exit code: %d\n", WEXITSTATUS(status)); 
				break;
      			}else
      			{
				sprintf(ret_string, "OK");
				 printf("wlxwlxrun shell script successfully.\n"); 
      			}
    		}else
		{
		      printf("exit status = [%d]\n", WEXITSTATUS(status));
		}
		//if (ret) {
			//LOGE("test %s failed with %d.\n", devName, ret);
			//sprintf(ret_string, "NG:test_failed");
			//break;
		//}

		//sprintf(ret_string, "OK");
	} while(0);

	send_response_packet(&info, ret_string, strlen(ret_string));
	return NULL;
}
