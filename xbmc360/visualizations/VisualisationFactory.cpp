#include "VisualisationFactory.h"
#include "utils\URIUtils.h"
#include "360Milkdrop2.h"

CVisualisationFactory::CVisualisationFactory()
{
}

CVisualisationFactory::~CVisualisationFactory()
{
}

CVisualisation* CVisualisationFactory::LoadVisualisation(const CStdString& strVisz) const
{
	// Strip of the path and extension to get the name of the visualisation
	CStdString strName = URIUtils::GetFileName(strVisz);
	strName = strName.Left(strName.size() - 4);

	// We are static linking as we don't have a DLL loader yet

	// We only have MilkDrop2 atm!

	CVisualisation* pVisualisation = NULL;
	pVisualisation = new CVisualisationMilkDrop2(strName);

	return pVisualisation;

	return NULL;
}
