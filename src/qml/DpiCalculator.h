/*!
 * \copyright Copyright (c) 2015-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include <QtGlobal>

namespace governikus
{

class DpiCalculator
{
	DpiCalculator()
	{
	}


	public:
		static qreal getDpi();
};

} /* namespace governikus */
