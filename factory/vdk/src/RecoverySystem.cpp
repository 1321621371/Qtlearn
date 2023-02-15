#include <stdio.h>
#include <cstring>
#include <errno.h>
#include <string.h>
#include <sys/reboot.h>
#include <unistd.h>
#include "RecoverySystem.h"

using namespace std;

typedef struct {
	char command[32];
	char status[32];
	char recovery[768];

	// The 'recovery' field used to be 1024 bytes.  It has only ever
	// been used to store the recovery command line, so 768 bytes
	// should be plenty.  We carve off the last 256 bytes to store the
	// stage string (for multistage packages) and possible future
	// expansion.
	char stage[32];
	char reserved[224];
} BootMsg_t;

static int setBootloaderMessage(const BootMsg_t *in)
{
	const char *miscDev = "/dev/mmcblk3p5";
	FILE* f = fopen("/dev/mmcblk3p5", "wb");
	if (f == NULL) {
		printf("Can't open %s\n(%s)\n", miscDev, strerror(errno));
		return -1;
	}

	int count = fwrite(in, sizeof(*in), 1, f);
	if (count != 1) {
		printf("Failed writing %s\n(%s)\n", miscDev, strerror(errno));
		return -1;
	}

	if (fclose(f) != 0) {
		printf("Failed closing %s\n(%s)\n", miscDev, strerror(errno));
		return -1;
	}

	return 0;
}

void RecoverySystem::installPackage(string packagePath, bool rebootNow)
{
	BootMsg_t bmsg;

	printf("%s\n", __func__);

	memset(&bmsg, 0, sizeof(BootMsg_t));

	strncpy(bmsg.command, "boot-recovery", sizeof(bmsg.command));
	strncpy(bmsg.recovery, "recovery\n", sizeof(bmsg.recovery));

	char args[128] = {0};
	memset(args, 0, sizeof(args));
	sprintf(args, "--update_package=%s\n", packagePath.c_str());
        strncat(bmsg.recovery, args, sizeof(args));

	setBootloaderMessage(&bmsg);
	sync();

	if (rebootNow) {
		reboot(RB_AUTOBOOT);
	}
}
