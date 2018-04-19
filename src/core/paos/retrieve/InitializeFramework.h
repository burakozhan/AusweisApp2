/*!
 * \brief Class to parse InitializeFramework from server.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "paos/ElementDetector.h"
#include "paos/PaosMessage.h"

namespace governikus
{

class InitializeFramework
	: public PaosMessage
	, public ElementDetector
{
	private:
		void parse();
		virtual bool handleFoundElement(const QString& pElementName, const QString& pValue, const QXmlStreamAttributes& pAttributes) override;

	public:
		InitializeFramework(const QByteArray& pXmlData);
};

} /* namespace governikus */
