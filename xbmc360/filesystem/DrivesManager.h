#ifndef CDRIVESMANAGER
#define CDRIVESMANAGER

#include "..\utils\StdString.h"
#include "Drive.h"
#include <vector>

class CDrivesManager
{
public:
	CDrivesManager();
	~CDrivesManager();

	void Init();
	bool MountAll();
	void Unmount();
	void getMountedDrives(std::vector<CDrive* const> *vecRetVal);
	CDrive* getDriveByExecutableSystemPath(CStdString strExecutableSystemPath);

private:
	CStdString getExecutablePathFromSystemPath(CStdString ExecutableSystemPath);

	std::vector<CDrive* const> m_vecDrives;
	CStdString m_strExecutableFilePath;
};

#endif //CDRIVESMANAGER