#ifndef __EVFACTORY_PCBA_CONFIG_PARSER_H_
#define __EVFACTORY_PCBA_CONFIG_PARSER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define PCBA_CONFIG_FILE     "/home/.factory/pcba_config.xml"

#define SECTION_ID_ATTR			"name"

#define SECTION_COMM			"section_comm"
#define SECTION_COMM_NET		"net"
#define SECTION_COMM_NET_IP		"ip"
#define SECTION_COMM_NET_DEVICE		"device"
#define SECTION_COMM_NET_SERVER_IP	"server_ip"
#define SECTION_COMM_COM		"com"

#define SECTION_DEVICES			"devices"
#define SECTION_DEVICES_DEVICE		"device"
#define SECTION_DEVICES_DEVICE_SYSFS	"sysfs"

#define SECTION_PCBA			"section_pcba"
#define SECTION_PCBA_ITEM		"item"

int config_parser_init(void);
void config_parser_exit(void);
int config_get_attr(const char *sectionName, const char *itemName,
                const char *idName, const char *attrName, char *retValue);

#ifdef __cplusplus
}
#endif

#endif
