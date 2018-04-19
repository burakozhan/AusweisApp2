/*!
 * \brief Command to perform the DID Authenticate EAC2 process.
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "asn1/CVCertificateChain.h"
#include "BaseCardCommand.h"

namespace governikus
{

class CardConnection;
class ChipAuthenticationInfo;

class DidAuthenticateEAC2Command
	: public BaseCardCommand
{
	Q_OBJECT

	private:
		CVCertificateChain mCvcChain;
		QString mEphemeralPublicKeyAsHex;
		QString mSignatureAsHex;
		QByteArray mAuthenticatedAuxiliaryDataAsBinary;
		QByteArray mEfCardSecurityAsHex;
		QByteArray mNonceAsHex;
		QByteArray mAuthTokenAsHex;

		CardReturnCode putCertificateChain(const CVCertificateChain& pCvcChain);
		CardReturnCode performTerminalAuthentication(const QByteArray& taProtocol,
				const QByteArray& chr,
				const QByteArray& auxiliaryData,
				const QByteArray& compressedEphemeralPublicKey,
				const QByteArray& signature);
		CardReturnCode performChipAuthentication(QSharedPointer<const ChipAuthenticationInfo> pChipAuthInfo,
				const QByteArray& ephemeralPublicKey,
				QByteArray& pNonceAsHex,
				QByteArray& pAuthTokenAsHex);

	protected:
		virtual void internalExecute() override;
		virtual ~DidAuthenticateEAC2Command() override;

	public:
		DidAuthenticateEAC2Command(QSharedPointer<CardConnectionWorker> pCardConnectionWorker,
				const CVCertificateChain& pCvcChain, const QString& pEphemeralPublicKeyAsHex,
				const QString& pSignatureAsHex, const QByteArray& pAuthenticatedAuxiliaryDataAsBinary);


		const QByteArray& getEfCardSecurityAsHex() const
		{
			return mEfCardSecurityAsHex;
		}


		const QByteArray& getNonceAsHex() const
		{
			return mNonceAsHex;
		}


		const QByteArray& getAuthTokenAsHex() const
		{
			return mAuthTokenAsHex;
		}


};

} /* namespace governikus */
