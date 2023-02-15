#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "common.h"
//#include "cutils/properties.h"
#include "config_parser.h"
#include "packet.h"
#include "test_item.h"
#include "Build.h"

void *get_firmware_version(void *param)
{
	char ret_string[128] = {0};
	char expectVer[16] = {0};
	//char firmware_version[PROPERTY_VALUE_MAX] = {0};
	struct packet_info info;

	LOGV("%s\n", __func__);
	memcpy(&info, param, sizeof(struct packet_info));

	do {
		int ret = config_get_attr(SECTION_PCBA, SECTION_PCBA_ITEM,
				"fwver", "version", expectVer);
		if (ret != 0) {
			LOGE("fwver not exist\n");
			sprintf(ret_string, "NG;fwver_not_exist");
			break;
		}

		// int osVer = Build::getOsSVNVersion();
		std::string osVer = Build::getOsVersion();

		if (expectVer == osVer)
			sprintf(ret_string, "OK;%s", osVer.c_str());
		else
			sprintf(ret_string, "NG;mismatch_%s_%s", osVer.c_str(), expectVer);
	} while (0);

	send_response_packet(&info, ret_string, strlen(ret_string));

	return NULL;
}
