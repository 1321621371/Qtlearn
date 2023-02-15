#include <aes.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <rpmb.h>
#include "Build.h"

using namespace std;

static const uint8_t aes_key[] = {
	0x1A, 0x8B, 0x1C, 0xAE, 0x58, 0xC3, 0xAD, 0x5C
};

static string& trim(string& s) {
	size_t startpos = s.find_first_not_of(" \t\r\n\v\f");
	if (string::npos != startpos)  {
		s = s.substr(startpos);
	}

	size_t endpos = s.find_last_not_of(" \t\r\n\v\f");
	if (string::npos != endpos) {
		s = s.substr(0, endpos + 1);
	}

	return s;
}

static string getConfig(string& key)
{
	fstream cfgFile;
	string value = "";

	cfgFile.open("/etc/build.prop");
	if (!cfgFile || !cfgFile.is_open()) {
		cout << "Failed to open config file" << endl;
		return value;
	}

	char buf[128];
	while(!cfgFile.eof()) {
		cfgFile.getline(buf, sizeof(buf));
		string line(buf);

		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;

		size_t pos = line.find('=');
		if (pos == string::npos)
			continue;

		string foundKey = line.substr(0, pos);
		if (foundKey != key)
			continue;

		value = line.substr(pos + 1);
		break;
        }

	cfgFile.close();
	return value;
}

int Build::getOsSVNVersion(void)
{
	string key = "ro.product.svn-version";
	string value = getConfig(key);
	if (value.empty())
		return 0;

	return atoi(value.c_str());
}

string Build::getOsVersion(void)
{
	string key = "ro.product.version";
	
	return getConfig(key);
}

int Build::getVdkVersion(void)
{
	return VDK_VER;
}

static string decrypt_sn(unsigned char *encrypted)
{
        aes_context aes;
        unsigned char sn[32];

        memset(&aes, 0, sizeof(aes_context));
        aes_set_key(&aes, aes_key, sizeof(aes_key) * 8);

        for (int i = 0; i < 2; i++) {
                aes_decrypt(&aes, encrypted + 17 + i * 16, sn + i * 16);
        }

        sn[31] = '\0';

	string ret;
	ret.assign(&sn[0], &sn[31]);
	return ret;
}

string Build::getSerial(void)
{
	int ret;
	uint8_t buffer[RPMB_BLOCK_SIZE];
	string bsn = "0000000000000000000000000000000";

	memset(buffer, 0, sizeof(buffer));

	char rpmbblk[32] = {0};

	do {
		ret = get_rpmbblk_path(rpmbblk);
		if (ret != 0) {
			cout << "get rpmb block path failed." << endl;
			break;
		}

		ret = do_rpmb_read_block(rpmbblk, RPMB_BLOCK_BSN, 1,
							buffer, NULL);
		if (ret != 0) {
			cout << "failed to read bsn" << endl;
			break;
		}

		bsn = decrypt_sn(buffer);
	} while (0);

	return bsn;
}
