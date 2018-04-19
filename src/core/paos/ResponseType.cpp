/*!
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include "ResponseType.h"

using namespace governikus;


ResponseType::ResponseType(PaosType pType)
	: PaosMessage(pType)
	, mResult(Result::createOk())
{
}


ResponseType::~ResponseType()
{
}


const Result& ResponseType::getResult() const
{
	return mResult;
}


void ResponseType::setResult(const Result& result)
{
	mResult = result;
}
