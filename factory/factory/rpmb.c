#include <stdio.h>
#include "common.h"
#include "rpmb.h"

int get_rpmbblk_path(char *retPath)
{
	if (!retPath)
		return -1;

	sprintf(retPath, "/dev/mmcblk1rpmb");
	return 0;
}

int try_read_rpmb_counter(char *err)
{
	int ret;
	char rpmbblk[32] = {0};

	ret = get_rpmbblk_path(rpmbblk);
	if (ret != 0) {
		LOGE("get rpmb block path failed.\n");
		if (err != NULL)
			sprintf(err, "NG;get_rpmb_path");

		return -1;
	}

	ret = do_rpmb_read_counter(rpmbblk);
	if (ret < 0) {
		LOGD("cannot read rpmb counter, write key first\n");
		ret = do_rpmb_write_key(rpmbblk, rmpb_key);
		if (ret != 0) {
			LOGE("write rpmb key failed\n");
			if (err != NULL)
				sprintf(err, "NG;rpmb_write_key");

			return ERROR_RPMB_WRITE_KEY;
		}

		// re-read rpmb counter to check if key is valid
		ret = do_rpmb_read_counter(rpmbblk);
		if (ret < 0) {
			LOGE("re-read rpmb counter failed\n");
			if (err != NULL)
				sprintf(err, "NG:rpmb_read_counter");

			return ERROR_RPMB_READ_COUNTER;
		}
	}

	return 0;
}
