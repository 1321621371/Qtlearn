#ifndef VDK_BUILD_H
#define VDK_BUILD_H

#include <string>

class Build
{
	public:
		static std::string getOsVersion(void);
		static int getOsSVNVersion(void);
		static int getVdkVersion(void);
		static std::string getSerial(void);

	private:
		static const int VDK_VER = 4;
};

#endif
