#ifndef _EVFACTORY_PCBA_RPMB_H_
#define _EVFACTORY_PCBA_RPMB_H_

#include "mmcwrapper.h"

#define RPMB_BLOCK_SIZE 256

#define RPMB_BLOCK_BSN 1

#define ERROR_RPMB_READ_COUNTER (-1)
#define ERROR_RPMB_WRITE_KEY    (-2)

static const uint8_t rmpb_key[32] = {
	0x84, 0x50, 0xC4, 0x04, 0x74, 0x76, 0x43, 0x51,
	0xA6, 0x82, 0x14, 0x6E, 0x3C, 0x82, 0x7F, 0xD9,
	0x44, 0x35, 0xB2, 0x5C, 0x07, 0xB5, 0x24, 0x0C,
	0xCA, 0xA3, 0xB0, 0x00, 0x16, 0x44, 0x7F, 0x25
};

int try_read_rpmb_counter(char *err);
int get_rpmbblk_path(char *retPath);

#endif
