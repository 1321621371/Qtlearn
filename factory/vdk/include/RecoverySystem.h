#ifndef VDK_RECOVERY_SYSTEM_H
#define VDK_RECOVERY_SYSTEM_H

#include <string>


class RecoverySystem
{
	public:
		static void installPackage(std::string packagePath, bool rebootNow);
};

#endif
