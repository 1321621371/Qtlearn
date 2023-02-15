#include <stdio.h>
#include <iostream>
#include "RecoverySystem.h"
#include "Build.h"

using namespace std;

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		cout << "Usage: test_vdk [cmd]\n\
	valid command:\n\
	svn-version\n\
	version\n\
	vdk-version\n\
	SN\n\
	update\n";
		return 0;
	}
	
	string cmd(argv[1]);

	if(cmd == "svn-version")
	{
		int osSVNVer = Build::getOsSVNVersion();
		printf("osSVNVer=%d\n", osSVNVer);
		return 0;
	}

	if(cmd == "version")
	{
		string osVer = Build::getOsVersion();
		printf("osVer=%s\n", osVer.c_str());
		return 0;
	}

	if(cmd == "vdk-version")
	{
		int vdkVer = Build::getVdkVersion();
		printf("vdkVer=%d\n", vdkVer);
		return 0;
	}

	if(cmd == "update")
	{
		RecoverySystem::installPackage("/home/root/.update/update.zip", false);
		cout << "installPackage in /home/root/.update/update.zip" << endl;
		return 0;
	}

	if(cmd == "SN")
	{
		std::string sn = Build::getSerial();
		cout << "SN: " << sn << endl;
	}

	return 0;
}
