#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "network.h"

int get_eth_link_state(const char *ethName)
{
	int ret = ETH_LINK_DOWN;
	char buffer[128];
	char fileName[128] = {0};
	FILE *file;

	if (!ethName) {
		return -1;
	}

	memset(buffer, 0 , sizeof(buffer));
	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "/sys/class/net/%s/operstate", ethName);
	file = fopen(fileName, "r");
	if (file == NULL) {
		LOGE("open %s failed.\n", fileName);
		return -1;
	}

	ret = fread(buffer, 1, sizeof(buffer), file);
	if (ret <= 0) {
		LOGE("read %s failed.\n", fileName);
		fclose(file);
		return -1;
	}
	fclose(file);

	buffer[strlen(buffer) -1] = '\0';
	LOGD("%s link state: %s\n", ethName, buffer);
	if (strncmp(buffer, "up", 2) == 0) {
		return  ETH_LINK_UP;
	} else if (strncmp(buffer, "down", 4) == 0) {
		return  ETH_LINK_DOWN;
	} else if (strncmp(buffer, "unknown", 7) == 0) {
		return  ETH_LINK_UNKNOWN;
	}

	return ETH_LINK_DOWN;
}

int get_eth_link_speed(const char *ethName)
{
	int ret;
	char buffer[128];
	char fileName[128] = {0};
	FILE *file;

	if (!ethName) {
		return -1;
	}

	memset(buffer, 0 , sizeof(buffer));
	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "/sys/class/net/%s/speed", ethName);
	file = fopen(fileName, "r");
	if (file == NULL) {
		LOGE("open %s failed.\n", fileName);
		return -1;
	}

	ret = fread(buffer, 1, sizeof(buffer), file);
	if (ret <= 0) {
		LOGE("read %s failed.\n", fileName);
		fclose(file);
		return -1;
	}
	fclose(file);

	buffer[strlen(buffer) -1] = '\0';
	LOGD("%s link speed: %s\n", ethName, buffer);
	return atoi(buffer);
}

int reset_eth_link(const char *ethName)
{
        char cmd[64] = {0};
        int ret;

        sprintf(cmd, "busybox ip link set dev %s down", ethName);
        ret = __system(cmd);
        if (ret != 0) {
                LOGW("set link down failed.\n");
        }

        sleep(1);

        sprintf(cmd, "busybox ip link set dev %s up", ethName);
        ret = __system(cmd);
        if (ret != 0) {
                LOGW("set link up failed.\n");
        }

        return 0;
}
