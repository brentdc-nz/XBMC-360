#pragma once

#include "utils\StdString.h"
#include "Visualisation.h"

class CVisualisationFactory
{
public:
	CVisualisationFactory();
	virtual ~CVisualisationFactory();
	CVisualisation* LoadVisualisation(const CStdString& strVisz) const;
};
