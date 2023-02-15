#include <errno.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config_parser.h"


static xmlDocPtr gXmlDocPtr = NULL;

/*
xml config:
<sectionName>
    <itemName name="idName" attrName="retValue"></item>
</sectionName>
*/
int config_get_attr(const char *sectionName, const char *itemName,
		const char *idName, const char *attrName, char *retValue)
{
	xmlNodePtr root, node;
	xmlChar *value;
	const xmlChar *idAttr = (xmlChar *)SECTION_ID_ATTR;

	if (!gXmlDocPtr || !sectionName || !itemName ||
			 !idName || !attrName || !retValue) {
		return -1;
	}

	root = xmlDocGetRootElement(gXmlDocPtr);
	for (node = root->children; node; node = node->next) {
		if (xmlStrcasecmp(node->name, BAD_CAST sectionName) == 0)
			break;
	}

	if (node == NULL) {
		LOGE("%s not found\n", sectionName);
		return -2;
	}

	for (node = node->children; node; node = node->next) {
		if (xmlStrcasecmp(node->name, BAD_CAST itemName) != 0)
			continue;

		value = xmlGetProp(node, idAttr);
		if (value == NULL) {
			LOGW("item without name id.\n");
			continue;
		}

		LOGV("check [%s]\n", value);
		if (strcmp((char*)value, idName)) {
			xmlFree(value);
			continue;
		}

		value = xmlGetProp(node, BAD_CAST attrName);
		if (value == NULL) {
			LOGE("attribute [%s] not exist.\n", attrName);
			return -2;
		}

		LOGV("found [%s]\n", value);
		strcpy(retValue, (char*)value);
		xmlFree(value);
		return 0;
	}

	LOGE("%s-%s not found\n", idName, attrName);
	return -2;
}

int config_parser_init(void) {
	char *content = NULL;
	long filesize;
	FILE *file;
	int ret = -1;

	file = fopen(PCBA_CONFIG_FILE, "r");
	if (file == NULL) {
		LOGE("Failed to open config file <%s>: %s\n",
				PCBA_CONFIG_FILE, strerror(errno));
		return -1;
	}

	fseek(file, 0, SEEK_END);
	filesize = ftell(file);
        rewind(file);

        content = (char *)malloc(filesize + 1);
	if (content == NULL) {
		LOGE("allocate memory %ld failed.\n", filesize + 1);
		fclose(file);
		return -1;
	}

        memset(content, 0, filesize + 1);
        ret = fread(content, 1, filesize, file);
	if (ret < filesize) {
		LOGE("read config file failed.\n");
		fclose(file);
		return -1;
	}

        fclose(file);
        LOGV("pcba config:\n%s\n",content);

	gXmlDocPtr = xmlParseMemory(content, filesize);
	if (gXmlDocPtr == NULL) {
		LOGE("parse xml in memory failed.\n");
		return -1;
	}

        return 0;
}

void config_parser_exit()
{
	if (gXmlDocPtr != NULL)
		xmlFreeDoc(gXmlDocPtr);
}
