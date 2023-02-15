#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"
#include "config_parser.h"
#include "network.h"
#include "test_item.h"
#include "udp.h"

static int copy_file(const char *srcFile, const char *dstFile)
{
	int inputFd, outputFd, openFlags;
	mode_t filePerms;
	ssize_t numRead;
	char buf[1024];

	if (!srcFile || !dstFile)
		return -1;

	/* Open input and output files */
	inputFd = open(srcFile, O_RDONLY);
	if (inputFd < 0) {
		LOGE("open %s failed.\n", srcFile);
		return -1;
	}

	openFlags = O_CREAT | O_WRONLY | O_TRUNC;
	filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
			S_IROTH | S_IWOTH;      /* rw-rw-rw- */
	outputFd = open(dstFile, openFlags, filePerms);
	if (outputFd < 0) {
		LOGE("open %s failed.\n", srcFile);
		close(inputFd);
		return -1;
	}

	while ((numRead = read(inputFd, buf, sizeof(buf))) > 0) {
		if (write(outputFd, buf, numRead) != numRead) {
			LOGE("write failed\n");
			close(inputFd);
			close(outputFd);
			return -1;
		}
	}

	if (numRead < 0) {
		LOGE("read failed\n");
		close(inputFd);
		close(outputFd);
		return -1;
	}

	close(inputFd);
	close(outputFd);
	return 0;
}

static inline char* to_lower(char *string)
{
	int i = 0;

	if (!string)
		return NULL;

	while(string[i]) {
		string[i] = tolower(string[i]);
		i++;
	}

	return string;
}

static int wait_link_ready(const char *ethName)
{
	int ret;
	char ipAddr[128] = {0};
	int count = 0;

	if (!ethName)
		return -1;

	ret = get_eth_link_state(ethName);
	if (ret < 0) {
		LOGE("get linkstate for %s failed.\n", ethName);
		return -2;
	}

	if (ret == ETH_LINK_UP) {
		return 0;
	}

	//TODO: NETLINK
	while(1) {
		ret = get_eth_link_state(ethName);
		if (ret == ETH_LINK_UP) {
			//link speed > 10Mbps
			ret = get_eth_link_speed(ethName);
			if (ret > 10) {
				return 0;
			}

			//TODO: negotiation
			reset_eth_link(ethName);
		} else if (ret == ETH_LINK_UNKNOWN) {
			if (count++ > 5) {
				reset_eth_link(ethName);
				count = 0;
			}
		}
		sleep(1);
	}

	return -2;
}

int getDiskFsType(char* devName, char *diskType)
{
	char cmd[100];

	sprintf(cmd, "fdisk -l | grep %s | grep NTFS", devName);
	LOGI("system:%s\n", cmd);
	if(system(cmd) == 0)
	{
		sprintf(diskType, "ntfs-3g");
		return 1;  //fuseblk(NTFS)
	}
	sprintf(cmd, "fdisk -l | grep %s | grep FAT", devName);
	LOGI("system:%s\n", cmd);
	if(system(cmd) == 0)
	{
		sprintf(diskType, "vfat");
		return 2;  //vfat
	}
	LOGE("unsupported device: %s\n", devName);
	return -1;
}

static int ensure_config_exists(const char *configName)
{
	DIR *dir;
	struct dirent* de;
	char devName[64] = {0};
	//const char *mountDir = "/tmp/storage";
	const char *mountDir = "/media/sd-mmcblk0p1";
	char configPath[128] = {0};
	int ret;
	char cmd[100];
	char diskFsType[20];

	dir = opendir(mountDir);
	if (dir != NULL) {
		closedir(dir);
		umount(mountDir);
	} else  if (errno == ENOENT) {
		ret = mkdir(mountDir, 0755);
		if (ret != 0) {
			LOGE("mkdir %s failed\n", mountDir);
			return -1;
		}
	} else {
		LOGE("open %s failed.\n", mountDir);
		return -1;
	}

	sprintf(configPath, "%s/%s", mountDir, configName);

	int retry = 10;
	while (retry-- > 0) {
		dir = opendir("/dev");
		if (dir == NULL) {
			LOGE("open /dev failed.\n");
			return -1;
		}

		while ((de = readdir(dir))) {
			LOGV("/dev/%s\n", de->d_name);
			if (strncmp(de->d_name, "sd", 2) != 0)
				continue;
			if(strlen(de->d_name) < 4)
				continue;

			sprintf(devName, "/dev/%s", de->d_name);
			LOGD("trying %s\n", devName);
			sprintf(cmd, "umount %s", devName);
			LOGI("system:%s\n", cmd);
			system(cmd);

			if(getDiskFsType(devName, diskFsType) < 0)
				return -1;

			sprintf(cmd, "mount -t %s %s %s", diskFsType, devName, mountDir);
			LOGI("system:%s\n", cmd);
			if(system(cmd) == 0)
			{
				LOGI("%s mounted at: %s\n", devName, mountDir);
				//mount success, check config file
				ret = access(configPath, F_OK);
				if (ret == 0) {
					closedir(dir);
					copy_file(configPath, PCBA_CONFIG_FILE);
					umount(mountDir);
					return 0;
				}

				umount(mountDir);
			}else{
				LOGI("mount %s ERROR\n", devName);
			}

		}

		closedir(dir);
		sleep(1);
	}

	return -2;
}

int main(int argc, char **argv)
{
	int ret, w;
	char configName[32] = {0};
	char cmd[128] = {0};

#if 1
	freopen("/home/.factory/log", "a", stdout);
	setbuf(stdout, NULL);
	freopen("/home/.factory/log", "a", stderr);
	setbuf(stderr, NULL);
#endif

	LOGI("start factory %s\n", FACTORY_VERSION);

	//search and backup configuration file
	//sprintf(configName, "mtu_factory.xml");
	//ret = ensure_config_exists(configName);
	//if (ret != 0) {
		//LOGE("No config file exists.\n");
		//return ret;
	//}

	//init config parser
	ret = config_parser_init();
	if (ret != 0) {
		LOGE("init config parser failed.\n");
		return ret;
	}

	char ip[128] = {0};
	char ethName[32] = {0};

	//setup primary eth ip
	ret = config_get_attr(SECTION_COMM, SECTION_COMM_NET, "primary",
						SECTION_COMM_NET_IP, ip);
	if (ret != 0) {
		LOGE("get %s from config failed.\n", SECTION_COMM_NET_IP);
		config_parser_exit();
		return ret;
	}

	ret = config_get_attr(SECTION_COMM, SECTION_COMM_NET, "primary",
					SECTION_COMM_NET_DEVICE, ethName);
	if (ret != 0) {
		LOGE("get %s from config failed.\n", SECTION_COMM_NET_DEVICE);
		config_parser_exit();
		return ret;
	}

	sprintf(cmd, "ip addr add %s/24 brd + dev %s &&"
			"busybox ip link set dev %s up", ip, ethName, ethName);
	ret = __system(cmd);
	if (ret != 0) {
		LOGE("setup ip(%s) failed with %d.\n", cmd, ret);
	}

	LOGI("IP: %s\n", ip);

	//ensure primary eth link up and link speed > 10Mbps
	ret = wait_link_ready(ethName);
	if (ret != 0) {
		LOGE("wait %s link up failed.\n", ethName);
		config_parser_exit();
		return ret;
	}

	//ping server
	ret = config_get_attr(SECTION_COMM, SECTION_COMM_NET, "primary",
					SECTION_COMM_NET_SERVER_IP, ip);
	if (ret == 0) {
		sprintf(cmd, "ping -I %s %s -w 5 -c 2", ethName, ip);
		ret = __system(cmd);
		if (ret != 0) {
			LOGW("ping server failed with %d.\n", ret);
		}
	} else {
		LOGW("get %s from config failed.\n", SECTION_COMM_NET_SERVER_IP);
	}


	//setup second eth ip
	ret = config_get_attr(SECTION_COMM, SECTION_COMM_NET, "second",
						SECTION_COMM_NET_IP, ip);
	if (ret == 0) {
		ret = config_get_attr(SECTION_COMM, SECTION_COMM_NET,
				"second", SECTION_COMM_NET_DEVICE, ethName);
		if (ret != 0) {
			LOGE("get %s from config failed.\n",
					SECTION_COMM_NET_DEVICE);
			config_parser_exit();
			return ret;
		}

		sprintf(cmd, "ip addr add %s/24 brd + dev %s &&"
				"busybox ip link set dev %s up",
					ip, ethName, ethName);
		__system(cmd);
	}

	create_udp_server();
	pthread_t udpRecvPid;
	pthread_create(&udpRecvPid, NULL, udp_recv_loop, NULL);

	pthread_join(udpRecvPid,NULL);
	destroy_udp_server();
	config_parser_exit();
	LOGE("end evfactory pcba\n");
        return 0;
}
