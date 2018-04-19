/*!
 * \brief Implementation of \ref ReaderManagerPlugIn for PCSC.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "PcscUtils.h"
#include "Reader.h"
#include "ReaderManagerPlugIn.h"

#include <QMap>


namespace governikus
{

class PcscReaderManagerPlugIn
	: public ReaderManagerPlugIn
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "governikus.ReaderManagerPlugIn" FILE "metadata.json")
	Q_INTERFACES(governikus::ReaderManagerPlugIn)

	private:
		SCARDCONTEXT mContextHandle;
		SCARD_READERSTATE mReaderState;
		int mTimerId;
		QMap<QString, Reader*> mReaders;

	private:
		PCSC_RETURNCODE readReaderNames(QStringList& pReaderNames);
		void updateReaders();
		void initReaderState();
		inline QString extractReaderName(PCSC_CHAR_PTR pReaderPointer);
		void removeReader(const QString& pReaderName);

	protected:
		void timerEvent(QTimerEvent* pEvent) override;

	public:
		PcscReaderManagerPlugIn();
		virtual ~PcscReaderManagerPlugIn() override;

		QList<Reader*> getReaders() const override;

		void init() override;
		void shutdown() override;
};

} /* namespace governikus */
