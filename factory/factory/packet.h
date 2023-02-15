#ifndef PACKET_H_
#define PACKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "udp.h"

#define PACKET_HEAD "*136"
#define PACKET_TAIL "#"

#define PACKET_TYPE_GET "GET"
#define PACKET_TYPE_SET "SET"
#define PACKET_TYPE_RET "RET"

#define DATA_SPLIT_TAG " "
#define PARAM_SPLIT_TAG ";"
#define TIMEOUT_VAL_TAG "TimeOut"

#define INDEX_PACKET_HEAD       0
#define INDEX_PACKET_TYPE       1
#define INDEX_PACKET_ITEM       2
#define INDEX_PACKET_UNIT       3

#define MAX_PACKET_CONTENT_LENGTH 6     /*命令中，以空格分开的字段最多6个*/
#define MIN_PACKET_CONTENT_LENGTH 5     /*命令中，以空格分开的字段最少5个*/

enum ITEM_TYPE {
        ITEM_TYPE_GET,
        ITEM_TYPE_SET,
        ITEM_TYPE_RET
};

struct packet_info {
        enum ITEM_TYPE type;    //测试类型 GET,SET
        int item_id;            //测试项在mapper中的序号
        int unit_id;            //unit参数
        char param[MAX_DATA_SIZE];      //测试参数
	unsigned int param_size;
        int timeout;            //超时时长
};


typedef void* (*cmd_handler)(void* param);

struct cmd_mapper {
        char *command;
        cmd_handler handler;
};


void handle_packet(char *packet, int size);
int send_response_packet(struct packet_info *info, char *message, int size);
int parse_timeout_value(char *string, int size);
int parse_param_value(char *in, int size, char *key, char *ret_string);
int get_unit_id(void);
int repack_content(char *packet,struct packet_info *info);
int repack_ret_content(char *packet,struct packet_info *info);
int parse_ret_packet(char *packet, int pack_size,struct packet_info *info);
char *get_cmd_by_itemid(int id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
