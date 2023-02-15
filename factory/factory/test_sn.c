#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "aes.h"
#include "common.h"
#include "packet.h"
#include "rpmb.h"
#include "test_item.h"

#define ROUND_UP(x, align) (((int)(x) + (align - 1)) & ~(align - 1))
#define BSN_LENGTH	(31)
#define ENCRY_BSN_BLOCK	(ROUND_UP(BSN_LENGTH, 16) / 16)

static const uint8_t aes_key[8] = {
	0x1A, 0x8B, 0x1C, 0xAE, 0x58, 0xC3, 0xAD, 0x5C
};

static void dump_data(const char *name, const unsigned char *buf,
						size_t buf_size)
{
	char tmpbuf[1024];
	char outputbuf[1024];
	size_t i;

	snprintf(tmpbuf, sizeof(tmpbuf), "%s[%u]", name, buf_size);

	for (i = 0; i < buf_size; i++) {
		if (i % 8 == 0 && i % 16) {
			snprintf(outputbuf, sizeof(outputbuf),
						"%s, ", tmpbuf);
			snprintf(tmpbuf, sizeof(tmpbuf), "%s", outputbuf);
		}
		snprintf(outputbuf, sizeof(outputbuf), "%s %02x",
						tmpbuf, buf[i]);
		snprintf(tmpbuf, sizeof(tmpbuf), "%s", outputbuf);
		if ((i + 1) % 16 == 0) {
			printf("%s\n", tmpbuf);
			snprintf(tmpbuf, sizeof(tmpbuf), "%s[%u]",
						name, buf_size);
		}
	}
	if (buf_size % 16) {
		printf("%s\n", tmpbuf);
	}
}

static void encrypt_sn(const char *bsn, unsigned char encrypted[256])
{
	struct timeval tm;
	aes_context aes;
	unsigned char buffer[16];
	unsigned char output[16];

        gettimeofday(&tm, NULL);
        srand((unsigned int)tm.tv_usec);
        for (int i = 0; i < 64; i++) {
                *(int *)&encrypted[i * 4] = rand();
        }

	memset(&aes, 0, sizeof(aes_context));
        aes_set_key(&aes, aes_key, sizeof(aes_key) * 8);
        size_t dataLeft = BSN_LENGTH;

        for (int i = 0; i < ENCRY_BSN_BLOCK; i ++) {
                int dataLen = (dataLeft > 16)? 16:dataLeft;
                memset(buffer, 0, sizeof(buffer));
                memcpy(buffer, bsn + i * 16, dataLen);

                aes_encrypt(&aes, buffer, output);
                //dump_data("encrypted output   ", output, 16);

                memcpy(encrypted + 17 + i * 16, output, 16);
                dataLeft -= dataLen;
        }
}

static void decrypt_sn(unsigned char encrypted[256], char *retString)
{
	aes_context aes;
	unsigned char buffer[16];
	unsigned char output[16];
	unsigned char sn[ENCRY_BSN_BLOCK * 16];

	memset(&aes, 0, sizeof(aes_context));
        aes_set_key(&aes, aes_key, sizeof(aes_key) * 8);

        for (int i = 0; i < ENCRY_BSN_BLOCK; i++) {
		memcpy(buffer, encrypted + 17 + i * 16, 16);
		//dump_data("encrypted buffer ", buffer, 16);
		aes_decrypt(&aes, buffer, output);
                memcpy(sn + i * 16, output, 16);
                //dump_data("dectyped sn  ", output, 16);
	}

	sn[BSN_LENGTH] = '\0';
	sprintf(retString, "OK;%s", sn);
}

static void do_handle_sn(struct packet_info *info)
{
	char ret_string[64] = {0};
	uint8_t buffer[RPMB_BLOCK_SIZE];
	int ret;
	uint16_t addr = RPMB_BLOCK_BSN;
	char rpmbblk[32] = {0};

	if (info == NULL) {
		return;
	}

	ret = get_rpmbblk_path(rpmbblk);
	if (ret != 0) {
		LOGE("get rpmb block path failed.\n");
		sprintf(ret_string, "NG;get_rpmb_path");
		send_response_packet(info, ret_string, strlen(ret_string));
		return;
	}

	memset(buffer, 0, sizeof(buffer));
	if (info->type == ITEM_TYPE_GET) {
		//get SN
		ret = do_rpmb_read_block(rpmbblk, addr, 1, buffer, rmpb_key);
                if (ret != 0) {
                        LOGE("failed to read sn\n");
                        sprintf(ret_string, "NG;rpmb_read");
                } else {
			decrypt_sn(buffer, ret_string);
                }
	} else if (info->type == ITEM_TYPE_SET) {
		//set SN
		if (strlen(info->param) != BSN_LENGTH) {
			LOGE("error check sn length: %u\n",
					strlen(info->param));
			sprintf(ret_string, "NG;err_sn_length");
			send_response_packet(info, ret_string,
					strlen(ret_string));
			return;
		}

		encrypt_sn(info->param, buffer);

		ret = try_read_rpmb_counter(ret_string);
		if (ret < 0) {
			send_response_packet(info, ret_string,
					strlen(ret_string));
			return;
		}

		ret = do_rpmb_write_block(rpmbblk, addr, buffer, rmpb_key);
		if (ret != 0) {
			LOGE("failed to write sn\n");
			sprintf(ret_string, "NG;rpmb_write_sn");
		} else {
			sprintf(ret_string, "OK;%s", info->param);
		}
	}

	send_response_packet(info, ret_string, strlen(ret_string));
}

void *handle_bsn(void *param)
{
	struct packet_info info;

	LOGV("%s\n", __func__);

	memcpy(&info, param, sizeof(struct packet_info));
	do_handle_sn(&info);

	return NULL;
}
