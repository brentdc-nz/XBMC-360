#ifndef CFACTORYDIRECTORY_H
#define CFACTORYDIRECTORY_H

#include "IDirectory.h"

namespace XFILE
{
// Get access to a directory of a file system.

// The Factory can be used to create a directory object
// for every file system accessable
// Example:
// CStdString strShare="iso9660://";
// IDirectory* pDir=CFactoryDirectory::Create(strShare);
// The \e pDir pointer can be used to access a directory and retrieve it's content

// When different types of shares have to be accessed use CVirtualDirectory
class CFactoryDirectory
{
public:
	static IDirectory* Create(const CStdString& strPath);
};
}

#endif//CFACTORYDIRECTORY_H
