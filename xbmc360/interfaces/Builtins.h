#ifndef H_CBUILTINS
#define H_CBUILTINS

#include "utils/StdString.h"

class CBuiltins
{
public:
	static bool HasCommand(const CStdString& execString);
	static void GetHelp(CStdString &help);
	static int Execute(const CStdString& execString);
};

#endif //H_CBUILTINS