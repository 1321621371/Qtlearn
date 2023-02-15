#ifndef _LIBRPMB_RPMB_H_
#define _LIBRPMB_RPMB_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RPMB_BLOCK_SIZE		(256)
#define RPMB_BLOCK_BSN		(1)

int get_rpmbblk_path(char *retPath);
int do_rpmb_read_block(const char *rpmbpath, uint16_t addr, uint16_t blocks_cnt, uint8_t *output, const uint8_t *key);

#ifdef __cplusplus
}
#endif

#endif //_LIBRPMB_RPMB_H_
