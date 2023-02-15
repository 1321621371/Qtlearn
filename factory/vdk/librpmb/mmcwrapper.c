#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include <linux/fs.h>

#include "mmcwrapper.h"
#include "hmac_sha2.h"

#define KEYLEN 32
#define DO_IO(func, fd, buf, nbyte)				\
	({							\
		ssize_t ret = 0, r;				\
		do {						\
			r = func(fd, buf + ret, nbyte - ret);	\
			if (r < 0 && errno != EINTR) {		\
				ret = -1;			\
				break;				\
			}					\
			else if (r > 0)				\
				ret += r;			\
		} while (r != 0 && (size_t)ret != nbyte);	\
								\
		ret;						\
	})

enum rpmb_op_type {
	MMC_RPMB_WRITE_KEY = 0x01,
	MMC_RPMB_READ_CNT  = 0x02,
	MMC_RPMB_WRITE     = 0x03,
	MMC_RPMB_READ      = 0x04,

	/* For internal usage only, do not use it directly */
	MMC_RPMB_READ_RESP = 0x05
};

struct rpmb_frame {
	u_int8_t  stuff[196];
	u_int8_t  key_mac[32];
	u_int8_t  data[256];
	u_int8_t  nonce[16];
	u_int32_t write_counter;
	u_int16_t addr;
	u_int16_t block_count;
	u_int16_t result;
	u_int16_t req_resp;
};

/* Performs RPMB operation.
 *
 * @fd: RPMB device on which we should perform ioctl command
 * @frame_in: input RPMB frame, should be properly inited
 * @frame_out: output (result) RPMB frame. Caller is responsible for checking
 *             result and req_resp for output frame.
 * @out_cnt: count of outer frames. Used only for multiple blocks reading,
 *           in the other cases -EINVAL will be returned.
 */
static int do_rpmb_op(int fd,
					  const struct rpmb_frame *frame_in,
					  struct rpmb_frame *frame_out,
					  unsigned int out_cnt)
{
	int err;
	u_int16_t rpmb_type;

	struct mmc_ioc_cmd ioc = {
		.arg        = 0x0,
		.blksz      = 512,
		.blocks     = 1,
		.write_flag = 1,
		.opcode     = MMC_WRITE_MULTIPLE_BLOCK,
		.flags      = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC,
		.data_ptr   = (uintptr_t)frame_in
	};

	if (!frame_in || !frame_out || !out_cnt)
		return -EINVAL;

	rpmb_type = be16toh(frame_in->req_resp);

	switch(rpmb_type) {
	case MMC_RPMB_READ:
		/* Request */
		err = ioctl(fd, MMC_IOC_CMD, &ioc);
		if (err < 0) {
			err = -errno;
			goto out;
		}

		/* Get response */
		ioc.write_flag = 0;
		ioc.opcode   = MMC_READ_MULTIPLE_BLOCK;
		ioc.blocks   = out_cnt;
		ioc.data_ptr = (uintptr_t)frame_out;
		err = ioctl(fd, MMC_IOC_CMD, &ioc);
		if (err < 0) {
			err = -errno;
			goto out;
		}

		break;
	case MMC_RPMB_WRITE:
	case MMC_RPMB_WRITE_KEY:
	case MMC_RPMB_READ_CNT:
	default:
		err = -EINVAL;
		goto out;
	}

out:
	return err;
}

int do_rpmb_read_block(const char *rpmbpath, uint16_t addr, uint16_t blocks_cnt, uint8_t *output, const uint8_t *key)
{
	int i, ret, dev_fd = -1;
	struct rpmb_frame frame_in = {
		.req_resp    = htobe16(MMC_RPMB_READ),
	}, *frame_out_p;

	if (output == NULL) {
		LOGE("no output buffer\n");
		return -1;
	}

	dev_fd = open(rpmbpath, O_RDONLY);
	if (dev_fd < 0) {
		LOGE("open failed: %s", strerror(errno));
		return -1;
	}

	/* Get block address */
	frame_in.addr = htobe16(addr);

	/* Get blocks count */
	if (!blocks_cnt) {
		LOGE("please, specify valid blocks count number\n");
		close(dev_fd);
		return -1;
	}

	frame_out_p = calloc(sizeof(*frame_out_p), blocks_cnt);
	if (!frame_out_p) {
		LOGE("can't allocate memory for RPMB outer frames\n");
		close(dev_fd);
		return -1;
	}

	/* Execute RPMB op */
	ret = do_rpmb_op(dev_fd, &frame_in, frame_out_p, blocks_cnt);
	if (ret != 0) {
		LOGE("ioctl failed: %s", strerror(errno));
		close(dev_fd);
		return -1;
	}

	/* Check RPMB response */
	if (frame_out_p[blocks_cnt - 1].result != 0) {
		LOGE("RPMB operation failed, retcode 0x%04x\n",
			   be16toh(frame_out_p[blocks_cnt - 1].result));
		close(dev_fd);
		return -1;
	}

	/* Do we have to verify data against key? */
	if (key) {
		unsigned char mac[32];
		hmac_sha256_ctx ctx;
		struct rpmb_frame *frame_out = NULL;

		hmac_sha256_init(&ctx, key, KEYLEN);
		for (i = 0; i < blocks_cnt; i++) {
			frame_out = &frame_out_p[i];
			hmac_sha256_update(&ctx, frame_out->data,
					sizeof(*frame_out) - offsetof(struct rpmb_frame, data));
		}

		hmac_sha256_final(&ctx, mac, sizeof(mac));

		/* Impossible */
		assert(frame_out);

		/* Compare calculated MAC and MAC from last frame */
		if (memcmp(mac, frame_out->key_mac, sizeof(mac))) {
			LOGE("RPMB MAC missmatch\n");
			close(dev_fd);
			return -1;
		}
	}

	/* Write data */
	for (i = 0; i < blocks_cnt; i++) {
		struct rpmb_frame *frame_out = &frame_out_p[i];
		memcpy(&output[i*256], frame_out->data, sizeof(frame_out->data));
	}

	free(frame_out_p);
	close(dev_fd);

	return ret;
}

int get_rpmbblk_path(char *retPath)
{
	if (!retPath)
		return -1;

	sprintf(retPath, "/dev/mmcblk3rpmb");
	return 0;
}
