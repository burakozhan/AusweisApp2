/*!
 * \brief Unit tests for \ref CertificateDescription
 *
 * \copyright Copyright (c) 2014-2018 Governikus GmbH & Co. KG, Germany
 */

#include <QtCore>
#include <QtTest>

#include "asn1/ASN1TemplateUtil.h"
#include "asn1/CertificateDescription.h"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/stack.h>

using namespace governikus;


namespace
{
const char* HEX_STRING("30 8202E0"
					   "        06 0A 04007F00070301030101"
					   "        A1 0E 0C0C442D547275737420476D6248"
					   "        A2 18 1316687474703A2F2F7777772E642D74727573742E6E6574"
					   "        A3 19 0C17476F7665726E696B757320476D6248202620436F2E4B47"
					   "        A4 1A 131868747470733A2F2F7777772E617574656E746170702E6465"
					   "        A5 820205"
					   "            0C 820201 4E616D652C20416E7363687269667420756E6420452D4D61696C2D4164726573736520646573204469656E737465616E626965746572733A0D0A476F7665726E696B757320476D6248202620436F2E4B470D0A416D2046616C6C7475726D20390D0A3238333539204272656D656E0D0A6B6F6E74616B7440676F7665726E696B75732E636F6D0D0A0D0A4765736368C3A46674737A7765636B3A0D0A53656C6273746175736B756E66740D0A0D0A48696E7765697320617566206469652066C3BC722064656E204469656E737465616E626965746572207A757374C3A46E646967656E205374656C6C656E2C20646965206469652045696E68616C74756E672064657220566F7273636872696674656E207A756D20446174656E73636875747A206B6F6E74726F6C6C696572656E3A0D0A446572204C616E64657362656175667472616774652066C3BC7220446174656E73636875747A20756E6420496E666F726D6174696F6E736672656968656974206465722046726569656E2048616E73657374616474204272656D656E0D0A41726E647473747261C39F6520310D0A3237353730204272656D6572686176656E0D0A303432312F3336312D323031300D0A6F666669636540646174656E73636875747A2E6272656D656E2E64650D0A687474703A2F2F7777772E646174656E73636875747A2E6272656D656E2E64650D0A"
					   "        A7 68"
					   "            31 66"
					   "                04 20 29F04459C39E4B68C70AA3961FA7B1B40E8A49238446CFE606C56FF8DC0989F4"
					   "                04 20 3FFF612368211F69D460DFF157F541BEBC96217D7AF6C2D4F57BF6E4A126A86C"
					   "                04 20 9D8EB0BDF36B19C4AF3147E57401FD792845F0413102A7BD784DF9418098BEBE"
					   "");

const char* SELF_AUTH_CERT_2017 = "308202e6060a04007f00070301030101a10e0c0c442d547275737420476d6248a2181316687474703a2f2f7777772e642d74727573742e6e6574a31a0c18476f7665726e696b757320476d6248202620436f2e204b47a41a131868747470733a2f2f7777772e617574656e746170702e6465a582020a0c820206efbbbf4e616d652c20416e7363687269667420756e6420452d4d61696c2d4164726573736520646573204469656e737465616e626965746572733a0d0a476f7665726e696b757320476d6248202620436f2e204b470d0a416d2046616c6c7475726d20390d0a3238333539204272656d656e0d0a6b6f6e74616b7440676f7665726e696b75732e636f6d0d0a0d0a4765736368c3a46674737a7765636b3a200d0a53656c6273746175736b756e66740d0a0d0a48696e7765697320617566206469652066c3bc722064656e204469656e737465616e626965746572207a757374c3a46e646967656e205374656c6c656e2c20646965206469652045696e68616c74756e672064657220566f7273636872696674656e207a756d20446174656e73636875747a206b6f6e74726f6c6c696572656e3a0d0a446965204c616e64657362656175667472616774652066c3bc7220446174656e73636875747a20756e6420496e666f726d6174696f6e736672656968656974206465722046726569656e2048616e73657374616474204272656d656e0d0a41726e647473747261c39f6520310d0a3237353730204272656d6572686176656e0d0a303432312f3539362d323031300d0a6f666669636540646174656e73636875747a2e6272656d656e2e64650d0a687474703a2f2f7777772e646174656e73636875747a2e6272656d656e2e64650d0aa76831660420a30a9a4617dc153926f731064043bba624b0cdd3b458ed8723c1cda33f1ffdd70420ab9fce5da4ba24d0b2664450fcced618f68fe9cbcdc4ee6e0bb0c59bd2aa86b60420fbf9f26b56b74cdf1c6e5cb1811bec1a8283a174c629b1974de17dc058b31bda";
}


class test_CertificateDescription
	: public QObject
{
	Q_OBJECT

	/*
	 * CertificateDescription ::= SEQUENCE {
	 *      descriptionType OBJECT IDENTIFIER,
	 *      issuerName [1] UTF8String,
	 *      issuerURL [2] PrintableString OPTIONAL,
	 *      subjectName [3] UTF8String,
	 *      subjectURL [4] PrintableString OPTIONAL,
	 *      termsOfUsage [5] ANY DEFINED BY descriptionType,
	 *      redirectURL [6] PrintableString OPTIONAL,
	 *      commCertificates [7] SET OF OCTET STRING OPTIONAL
	 * }
	 */

	private Q_SLOTS:
		void parseCrap()
		{
			QByteArray hexString("30 8202A4");

			auto certDescr = CertificateDescription::fromHex(hexString);

			QVERIFY(certDescr == nullptr);
		}


		void missingMandatoryElement()
		{
			QByteArray hexString("30 820298"
								 "        A1 0E 0C0C442D547275737420476D6248"
								 "        A3 3A 0C38476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E"
								 "        A5 820248"
								 "            04 820244 4E616D652C20416E7363687269667420756E6420452D4D61696C2D4164726573736520646573204469656E737465616E626965746572733A0D0A476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E0D0A57696C68656C6D73747261C39F652034332F3433670D0A3130313137204265726C696E0D0A6265726C696E406764762E64650D0A0D0A4765736368C3A46674737A7765636B3A0D0A2D52656769737472696572756E6720756E64204C6F67696E20616D204744562D4D616B6C6572706F7274616C2D0D0A0D0A48696E7765697320617566206469652066C3BC722064656E204469656E737465616E626965746572207A757374C3A46E646967656E205374656C6C656E2C20646965206469652045696E68616C74756E672064657220566F7273636872696674656E207A756D20446174656E73636875747A206B6F6E74726F6C6C696572656E3A0D0A4265726C696E6572204265617566747261677465722066C3BC7220446174656E73636875747A20756E6420496E666F726D6174696F6E7366726569686569740D0A416E20646572205572616E696120342D31300D0A3130373837204265726C696E0D0A3033302F3133382038392D300D0A6D61696C626F7840646174656E73636875747A2D6265726C696E2E64650D0A687474703A2F2F7777772E646174656E73636875747A2D6265726C696E2E64650D0A416E737072656368706172746E65723A2044722E20416C6578616E64657220446978"
								 "");

			auto certDescr = CertificateDescription::fromHex(hexString);

			QVERIFY(certDescr == nullptr);
		}


		void onlyMandatoryFields()
		{
			QByteArray hexString("30 8202A4"
								 "        06 0A 04007F00070301030103"
								 "        A1 0E 0C0C442D547275737420476D6248"
								 "        A3 3A 0C38476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E"
								 "        A5 820248"
								 "            04 820244 4E616D652C20416E7363687269667420756E6420452D4D61696C2D4164726573736520646573204469656E737465616E626965746572733A0D0A476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E0D0A57696C68656C6D73747261C39F652034332F3433670D0A3130313137204265726C696E0D0A6265726C696E406764762E64650D0A0D0A4765736368C3A46674737A7765636B3A0D0A2D52656769737472696572756E6720756E64204C6F67696E20616D204744562D4D616B6C6572706F7274616C2D0D0A0D0A48696E7765697320617566206469652066C3BC722064656E204469656E737465616E626965746572207A757374C3A46E646967656E205374656C6C656E2C20646965206469652045696E68616C74756E672064657220566F7273636872696674656E207A756D20446174656E73636875747A206B6F6E74726F6C6C696572656E3A0D0A4265726C696E6572204265617566747261677465722066C3BC7220446174656E73636875747A20756E6420496E666F726D6174696F6E7366726569686569740D0A416E20646572205572616E696120342D31300D0A3130373837204265726C696E0D0A3033302F3133382038392D300D0A6D61696C626F7840646174656E73636875747A2D6265726C696E2E64650D0A687474703A2F2F7777772E646174656E73636875747A2D6265726C696E2E64650D0A416E737072656368706172746E65723A2044722E20416C6578616E64657220446978"
								 "");

			auto certDescr = CertificateDescription::fromHex(hexString);

			QVERIFY(certDescr != nullptr);
			QCOMPARE(certDescr->getDescriptionType(), QByteArray("0.4.0.127.0.7.3.1.3.1.3"));
			QCOMPARE(certDescr->getIssuerName(), QString("D-Trust GmbH"));
			QCOMPARE(certDescr->getIssuerUrl(), QString());
			QCOMPARE(certDescr->getSubjectName(), QString("Gesamtverband der deutschen Versicherungswirtschaft e.V."));
			QCOMPARE(certDescr->getSubjectUrl(), QString());
			QCOMPARE(certDescr->getTermsOfUsageType(), CertificateDescription::TermsOfUsageType::PDF);
			QCOMPARE(certDescr->getTermsOfUsage(), QString("048202444E616D652C20416E7363687269667420756E6420452D4D61696C2D4164726573736520646573204469656E737465616E626965746572733A0D0A476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E0D0A57696C68656C6D73747261C39F652034332F3433670D0A3130313137204265726C696E0D0A6265726C696E406764762E64650D0A0D0A4765736368C3A46674737A7765636B3A0D0A2D52656769737472696572756E6720756E64204C6F67696E20616D204744562D4D616B6C6572706F7274616C2D0D0A0D0A48696E7765697320617566206469652066C3BC722064656E204469656E737465616E626965746572207A757374C3A46E646967656E205374656C6C656E2C20646965206469652045696E68616C74756E672064657220566F7273636872696674656E207A756D20446174656E73636875747A206B6F6E74726F6C6C696572656E3A0D0A4265726C696E6572204265617566747261677465722066C3BC7220446174656E73636875747A20756E6420496E666F726D6174696F6E7366726569686569740D0A416E20646572205572616E696120342D31300D0A3130373837204265726C696E0D0A3033302F3133382038392D300D0A6D61696C626F7840646174656E73636875747A2D6265726C696E2E64650D0A687474703A2F2F7777772E646174656E73636875747A2D6265726C696E2E64650D0A416E737072656368706172746E65723A2044722E20416C6578616E64657220446978"));
			QCOMPARE(certDescr->getRedirectUrl(), QString());
			QCOMPARE(certDescr->getCommCertificates().size(), 0);
		}


		void allFieldsPresent()
		{
			QByteArray hexString("30 82038D"
								 "        06 0A 04007F00070301030103"
								 "        A1 0E 0C0C442D547275737420476D6248"
								 "        A2 18 1316687474703A2F2F7777772E642D74727573742E6E6574"
								 "        A3 3A 0C38476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E"
								 "        A4 1F 131D68747470733A2F2F7777772E6764762D74657374706F7274616C2E6465"
								 "        A5 820248"
								 "            04 820244 4E616D652C20416E7363687269667420756E6420452D4D61696C2D4164726573736520646573204469656E737465616E626965746572733A0D0A476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E0D0A57696C68656C6D73747261C39F652034332F3433670D0A3130313137204265726C696E0D0A6265726C696E406764762E64650D0A0D0A4765736368C3A46674737A7765636B3A0D0A2D52656769737472696572756E6720756E64204C6F67696E20616D204744562D4D616B6C6572706F7274616C2D0D0A0D0A48696E7765697320617566206469652066C3BC722064656E204469656E737465616E626965746572207A757374C3A46E646967656E205374656C6C656E2C20646965206469652045696E68616C74756E672064657220566F7273636872696674656E207A756D20446174656E73636875747A206B6F6E74726F6C6C696572656E3A0D0A4265726C696E6572204265617566747261677465722066C3BC7220446174656E73636875747A20756E6420496E666F726D6174696F6E7366726569686569740D0A416E20646572205572616E696120342D31300D0A3130373837204265726C696E0D0A3033302F3133382038392D300D0A6D61696C626F7840646174656E73636875747A2D6265726C696E2E64650D0A687474703A2F2F7777772E646174656E73636875747A2D6265726C696E2E64650D0A416E737072656368706172746E65723A2044722E20416C6578616E64657220446978"
								 "        A6 1E 131C68747470733a2f2f7777772e72656469726563742d746573742e6465"
								 "        A7 818B"
								 "            31 8188"
								 "                04 20 0617A64C37179860FEA3044A14052505FB4F8EF4D61F047B6A30130CD06718E6"
								 "                04 20 718FA21C2DF3A81BEF1B7A53360481C78C017F1EC7170266C2C4E9BB654B0E3B"
								 "                04 20 94B0AA7E8114F3E6DFCD52DA9F43E8B13CCB0589B8957E364728198FB4971AE6"
								 "                04 20 E85E1E8A78864E9246C86CF1C2A3810603EEEE75746C70CD51ACB86B5E2655D8"
								 "");

			auto certDescr = CertificateDescription::fromHex(hexString);

			QVERIFY(certDescr != nullptr);
			QCOMPARE(certDescr->getDescriptionType(), QByteArray("0.4.0.127.0.7.3.1.3.1.3"));
			QCOMPARE(certDescr->getIssuerName(), QString("D-Trust GmbH"));
			QCOMPARE(certDescr->getIssuerUrl(), QString("http://www.d-trust.net"));
			QCOMPARE(certDescr->getSubjectName(), QString("Gesamtverband der deutschen Versicherungswirtschaft e.V."));
			QCOMPARE(certDescr->getSubjectUrl(), QString("https://www.gdv-testportal.de"));
			QCOMPARE(certDescr->getTermsOfUsageType(), CertificateDescription::TermsOfUsageType::PDF);
			QCOMPARE(certDescr->getTermsOfUsage(), QString("048202444E616D652C20416E7363687269667420756E6420452D4D61696C2D4164726573736520646573204469656E737465616E626965746572733A0D0A476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E0D0A57696C68656C6D73747261C39F652034332F3433670D0A3130313137204265726C696E0D0A6265726C696E406764762E64650D0A0D0A4765736368C3A46674737A7765636B3A0D0A2D52656769737472696572756E6720756E64204C6F67696E20616D204744562D4D616B6C6572706F7274616C2D0D0A0D0A48696E7765697320617566206469652066C3BC722064656E204469656E737465616E626965746572207A757374C3A46E646967656E205374656C6C656E2C20646965206469652045696E68616C74756E672064657220566F7273636872696674656E207A756D20446174656E73636875747A206B6F6E74726F6C6C696572656E3A0D0A4265726C696E6572204265617566747261677465722066C3BC7220446174656E73636875747A20756E6420496E666F726D6174696F6E7366726569686569740D0A416E20646572205572616E696120342D31300D0A3130373837204265726C696E0D0A3033302F3133382038392D300D0A6D61696C626F7840646174656E73636875747A2D6265726C696E2E64650D0A687474703A2F2F7777772E646174656E73636875747A2D6265726C696E2E64650D0A416E737072656368706172746E65723A2044722E20416C6578616E64657220446978"));
			QCOMPARE(certDescr->getRedirectUrl(), QString("https://www.redirect-test.de"));
			QCOMPARE(certDescr->getCommCertificates().size(), 4);
			QVERIFY(certDescr->getCommCertificates().contains(QString("0617A64C37179860FEA3044A14052505FB4F8EF4D61F047B6A30130CD06718E6")));
			QVERIFY(certDescr->getCommCertificates().contains(QString("718FA21C2DF3A81BEF1B7A53360481C78C017F1EC7170266C2C4E9BB654B0E3B")));
			QVERIFY(certDescr->getCommCertificates().contains(QString("94B0AA7E8114F3E6DFCD52DA9F43E8B13CCB0589B8957E364728198FB4971AE6")));
			QVERIFY(certDescr->getCommCertificates().contains(QString("E85E1E8A78864E9246C86CF1C2A3810603EEEE75746C70CD51ACB86B5E2655D8")));
		}


		void termsOfUsagePdf()
		{
			QByteArray hexString("30 8202A4"
								 "        06 0A 04007F00070301030103"
								 "        A1 0E 0C0C442D547275737420476D6248"
								 "        A3 3A 0C38476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E"
								 "        A5 820248"
								 "            04 820244 4E616D652C20416E7363687269667420756E6420452D4D61696C2D4164726573736520646573204469656E737465616E626965746572733A0D0A476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E0D0A57696C68656C6D73747261C39F652034332F3433670D0A3130313137204265726C696E0D0A6265726C696E406764762E64650D0A0D0A4765736368C3A46674737A7765636B3A0D0A2D52656769737472696572756E6720756E64204C6F67696E20616D204744562D4D616B6C6572706F7274616C2D0D0A0D0A48696E7765697320617566206469652066C3BC722064656E204469656E737465616E626965746572207A757374C3A46E646967656E205374656C6C656E2C20646965206469652045696E68616C74756E672064657220566F7273636872696674656E207A756D20446174656E73636875747A206B6F6E74726F6C6C696572656E3A0D0A4265726C696E6572204265617566747261677465722066C3BC7220446174656E73636875747A20756E6420496E666F726D6174696F6E7366726569686569740D0A416E20646572205572616E696120342D31300D0A3130373837204265726C696E0D0A3033302F3133382038392D300D0A6D61696C626F7840646174656E73636875747A2D6265726C696E2E64650D0A687474703A2F2F7777772E646174656E73636875747A2D6265726C696E2E64650D0A416E737072656368706172746E65723A2044722E20416C6578616E64657220446978"
								 "");

			auto certDescr = CertificateDescription::fromHex(hexString);

			QVERIFY(certDescr != nullptr);
			QCOMPARE(certDescr->getTermsOfUsageType(), CertificateDescription::TermsOfUsageType::PDF);
			QCOMPARE(certDescr->getTermsOfUsage(), QString("048202444E616D652C20416E7363687269667420756E6420452D4D61696C2D4164726573736520646573204469656E737465616E626965746572733A0D0A476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E0D0A57696C68656C6D73747261C39F652034332F3433670D0A3130313137204265726C696E0D0A6265726C696E406764762E64650D0A0D0A4765736368C3A46674737A7765636B3A0D0A2D52656769737472696572756E6720756E64204C6F67696E20616D204744562D4D616B6C6572706F7274616C2D0D0A0D0A48696E7765697320617566206469652066C3BC722064656E204469656E737465616E626965746572207A757374C3A46E646967656E205374656C6C656E2C20646965206469652045696E68616C74756E672064657220566F7273636872696674656E207A756D20446174656E73636875747A206B6F6E74726F6C6C696572656E3A0D0A4265726C696E6572204265617566747261677465722066C3BC7220446174656E73636875747A20756E6420496E666F726D6174696F6E7366726569686569740D0A416E20646572205572616E696120342D31300D0A3130373837204265726C696E0D0A3033302F3133382038392D300D0A6D61696C626F7840646174656E73636875747A2D6265726C696E2E64650D0A687474703A2F2F7777772E646174656E73636875747A2D6265726C696E2E64650D0A416E737072656368706172746E65723A2044722E20416C6578616E64657220446978"));
		}


		void termsOfUsageHtml()
		{
			QByteArray hexString("30 820151"
								 "        06 0A 04007F00070301030102"
								 "        A1 16 0C14476F7665726E696B757320546573742044564341"
								 "        A2 1A 1318687474703A2F2F7777772E676F7665726E696B75732E6465"
								 "        A3 0C 0C0A476F7665726E696B7573"
								 "        A4 29 132768747470733A2F2F6465762D64656D6F2E676F7665726E696B75732D6569642E64653A38343433"
								 "        A5 81AF"
								 "            16 81AC 3C68313E43657274696669636174654465736372697074696F6E206D69742048544D4C3C2F68313E0A3C703E0A44696573206973742065696E20546573742C207A7572202655756D6C3B62657270722675756D6C3B66756E672065696E65722043657274696669636174654465736372697074696F6E206D69742048544D4C2D456C656D656E74656E2E0A3C2F703E0A3C703E0A3C656D3E4B6C6170707427733F3C2F656D3E0A3C2F703E0A"
								 "        A7 24"
								 "            31 22"
								 "                04 20 761099A58BFD5334E93A7A78E4F18B760FFCF8F513A4730C8AE9B59BCC0FE8C9"
								 "");

			QString termsOfUsage("<h1>CertificateDescription mit HTML</h1>\n"
								 "<p>\n"
								 "Dies ist ein Test, zur &Uuml;berpr&uuml;fung einer CertificateDescription mit HTML-Elementen.\n"
								 "</p>\n"
								 "<p>\n"
								 "<em>Klappt's?</em>\n"
								 "</p>\n");

			auto certDescr = CertificateDescription::fromHex(hexString);

			QVERIFY(certDescr != nullptr);
			QCOMPARE(certDescr->getTermsOfUsageType(), CertificateDescription::TermsOfUsageType::HTML);
			QCOMPARE(certDescr->getTermsOfUsage(), termsOfUsage);
		}


		void termsOfUsagePlain()
		{
			// "\r\n" isn't normalized to "\n" because getTermsOfUsage() returns the raw data
			QString termosOfUsage = QStringLiteral("Name, Anschrift und E-Mail-Adresse des Diensteanbieters:\r\n"
												   "Governikus GmbH & Co.KG\r\n"
												   "Am Fallturm 9\r\n"
												   "28359 Bremen\r\n"
												   "kontakt@governikus.com\r\n"
												   "\r\n"
												   "Gesch\u00E4ftszweck:\r\n"
												   "Selbstauskunft\r\n"
												   "\r\n"
												   "Hinweis auf die f\u00FCr den Diensteanbieter zust\u00E4ndigen Stellen, die die Einhaltung der Vorschriften zum Datenschutz kontrollieren:\r\n"
												   "Der Landesbeauftragte f\u00FCr Datenschutz und Informationsfreiheit der Freien Hansestadt Bremen\r\n"
												   "Arndtstra\u00DFe 1\r\n"
												   "27570 Bremerhaven\r\n"
												   "0421/361-2010\r\n"
												   "office@datenschutz.bremen.de\r\n"
												   "http://www.datenschutz.bremen.de\r\n");

			auto certDescr = CertificateDescription::fromHex(HEX_STRING);

			QVERIFY(certDescr != nullptr);
			QCOMPARE(certDescr->getTermsOfUsageType(), CertificateDescription::TermsOfUsageType::PLAIN_TEXT);
			QCOMPARE(certDescr->getTermsOfUsage(), termosOfUsage);
		}


		void providerAddressFromTermsOfUsage()
		{
			const QString providerAddress("Governikus GmbH & Co.KG\n"
										  "Am Fallturm 9\n"
										  "28359 Bremen\n"
										  "kontakt@governikus.com");

			auto certDescr = CertificateDescription::fromHex(HEX_STRING);
			QVERIFY(certDescr);
			QCOMPARE(certDescr->getServiceProviderAddress(), providerAddress);

			const QString providerAddress2017("Governikus GmbH & Co. KG\n"
											  "Am Fallturm 9\n"
											  "28359 Bremen\n"
											  "kontakt@governikus.com");
			certDescr = CertificateDescription::fromHex(SELF_AUTH_CERT_2017);
			QVERIFY(certDescr);
			QCOMPARE(certDescr->getServiceProviderAddress(), providerAddress2017);

		}


		void purposeFromTermsOfUsage()
		{
			const QString expectedPurpose("Selbstauskunft");

			auto certDescr = CertificateDescription::fromHex(HEX_STRING);
			QVERIFY(certDescr);
			QCOMPARE(certDescr->getPurpose(), expectedPurpose);

			certDescr = CertificateDescription::fromHex(SELF_AUTH_CERT_2017);
			QVERIFY(certDescr);
			QCOMPARE(certDescr->getPurpose(), expectedPurpose);
		}


		void dataSecurityOfficerFromTermsOfUsage()
		{
			const QString dataSecurityOfficer = QStringLiteral("Der Landesbeauftragte f\u00FCr Datenschutz und Informationsfreiheit der Freien Hansestadt Bremen\n"
															   "Arndtstra\u00DFe 1\n"
															   "27570 Bremerhaven\n"
															   "0421/361-2010\n"
															   "office@datenschutz.bremen.de\n"
															   "http://www.datenschutz.bremen.de");

			auto certDescr = CertificateDescription::fromHex(HEX_STRING);
			QVERIFY(certDescr);
			QCOMPARE(certDescr->getDataSecurityOfficer(), dataSecurityOfficer);

			const QString dataSecurityOfficer2017 = QStringLiteral("Die Landesbeauftragte f\u00FCr Datenschutz und Informationsfreiheit der Freien Hansestadt Bremen\n"
																   "Arndtstra\u00DFe 1\n"
																   "27570 Bremerhaven\n"
																   "0421/596-2010\n"
																   "office@datenschutz.bremen.de\n"
																   "http://www.datenschutz.bremen.de");

			certDescr = CertificateDescription::fromHex(SELF_AUTH_CERT_2017);
			QVERIFY(certDescr);
			QCOMPARE(certDescr->getDataSecurityOfficer(), dataSecurityOfficer2017);
		}


		void opensslEncode()
		{
			CertificateDescription* certDescr = nullptr;
			if (!(certDescr = CertificateDescription_new()))
			{
				BIO* bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
				ERR_print_errors(bio_err);
				BIO_free(bio_err);
				return;
			}


			certDescr->setDescriptionType("0.4.0.127.0.7.3.1.3.1.3");
			certDescr->setIssuerName("D-Trust GmbH");
			certDescr->setIssuerUrl("http://www.d-trust.net");
			certDescr->setSubjectName("Gesamtverband der deutschen Versicherungswirtschaft e.V.");
			certDescr->setSubjectUrl("https://www.gdv-testportal.de");
			certDescr->setRedirectUrl("https://www.redirect-test.de");

			QByteArray termsOfUsageBytes = QByteArray::fromHex("4e616d652c20416e7363687269667420756e6420452d4d61696c2d4164726573736520646573204469656e737465616e626965746572733a0d0a476573616d7476657262616e64206465722064657574736368656e20566572736963686572756e67737769727473636861667420652e562e0d0a57696c68656c6d73747261c39f652034332f3433670d0a3130313137204265726c696e0d0a6265726c696e406764762e64650d0a0d0a4765736368c3a46674737a7765636b3a0d0a2d52656769737472696572756e6720756e64204c6f67696e20616d204744562d4d616b6c6572706f7274616c2d0d0a0d0a48696e7765697320617566206469652066c3bc722064656e204469656e737465616e626965746572207a757374c3a46e646967656e205374656c6c656e2c20646965206469652045696e68616c74756e672064657220566f7273636872696674656e207a756d20446174656e73636875747a206b6f6e74726f6c6c696572656e3a0d0a4265726c696e6572204265617566747261677465722066c3bc7220446174656e73636875747a20756e6420496e666f726d6174696f6e7366726569686569740d0a416e20646572205572616e696120342d31300d0a3130373837204265726c696e0d0a3033302f3133382038392d300d0a6d61696c626f7840646174656e73636875747a2d6265726c696e2e64650d0a687474703a2f2f7777772e646174656e73636875747a2d6265726c696e2e64650d0a416e737072656368706172746e65723a2044722e20416c6578616e64657220446978");
			ASN1_TYPE_set_octetstring(certDescr->mTermsOfUsage, reinterpret_cast<unsigned char*>(termsOfUsageBytes.data()), termsOfUsageBytes.length());
			{
				unsigned char buf[1024];
				int buf_len = ASN1_TYPE_get_octetstring(certDescr->mTermsOfUsage, buf, 1024);
				if (buf_len == 1024)
				{
					qWarning() << "Possibly the buffer is too short";
				}
				QByteArray byteBuf(reinterpret_cast<char*>(buf), buf_len);
				QVERIFY(byteBuf.contains("Name, Anschrift und E-Mail-Adresse des Diensteanbieters:"));
			}

#if OPENSSL_VERSION_NUMBER < 0x10100000L
			certDescr->mCommCertificates = SKM_sk_new(ASN1_OCTET_STRING, 0);
#else
			certDescr->mCommCertificates = sk_ASN1_OCTET_STRING_new(0);
#endif
			QByteArrayList commCertBytes;
			commCertBytes.append(QByteArray::fromHex("94B0AA7E8114F3E6DFCD52DA9F43E8B13CCB0589B8957E364728198FB4971AE6"));
			commCertBytes.append(QByteArray::fromHex("E85E1E8A78864E9246C86CF1C2A3810603EEEE75746C70CD51ACB86B5E2655D8"));
			commCertBytes.append(QByteArray::fromHex("0617A64C37179860FEA3044A14052505FB4F8EF4D61F047B6A30130CD06718E6"));
			commCertBytes.append(QByteArray::fromHex("718FA21C2DF3A81BEF1B7A53360481C78C017F1EC7170266C2C4E9BB654B0E3B"));
			for (const auto& commCertByte : qAsConst(commCertBytes))
			{
				ASN1_OCTET_STRING* octetString = ASN1_OCTET_STRING_new();
				ASN1_OCTET_STRING_set(octetString, reinterpret_cast<const unsigned char*>(commCertByte.constData()), commCertByte.length());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
				SKM_sk_push(ASN1_OCTET_STRING, certDescr->mCommCertificates, octetString);
#else
				sk_ASN1_OCTET_STRING_push(certDescr->mCommCertificates, octetString);
#endif
			}
			{
				for (int i = 0; i < sk_ASN1_OCTET_STRING_num(certDescr->mCommCertificates); i++)
				{
					ASN1_OCTET_STRING* octetString = sk_ASN1_OCTET_STRING_value(certDescr->mCommCertificates, i);
					QByteArray byteBuf(reinterpret_cast<char*>(octetString->data), octetString->length);
					QVERIFY(!byteBuf.isEmpty());
				}
			}


			QByteArray hexString("30 82038D"
								 "        06 0A 04007F00070301030103"
								 "        A1 0E 0C0C442D547275737420476D6248"
								 "        A2 18 1316687474703A2F2F7777772E642D74727573742E6E6574"
								 "        A3 3A 0C38476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E"
								 "        A4 1F 131D68747470733A2F2F7777772E6764762D74657374706F7274616C2E6465"
								 "        A5 820248"
								 "            04 820244 4E616D652C20416E7363687269667420756E6420452D4D61696C2D4164726573736520646573204469656E737465616E626965746572733A0D0A476573616D7476657262616E64206465722064657574736368656E20566572736963686572756E67737769727473636861667420652E562E0D0A57696C68656C6D73747261C39F652034332F3433670D0A3130313137204265726C696E0D0A6265726C696E406764762E64650D0A0D0A4765736368C3A46674737A7765636B3A0D0A2D52656769737472696572756E6720756E64204C6F67696E20616D204744562D4D616B6C6572706F7274616C2D0D0A0D0A48696E7765697320617566206469652066C3BC722064656E204469656E737465616E626965746572207A757374C3A46E646967656E205374656C6C656E2C20646965206469652045696E68616C74756E672064657220566F7273636872696674656E207A756D20446174656E73636875747A206B6F6E74726F6C6C696572656E3A0D0A4265726C696E6572204265617566747261677465722066C3BC7220446174656E73636875747A20756E6420496E666F726D6174696F6E7366726569686569740D0A416E20646572205572616E696120342D31300D0A3130373837204265726C696E0D0A3033302F3133382038392D300D0A6D61696C626F7840646174656E73636875747A2D6265726C696E2E64650D0A687474703A2F2F7777772E646174656E73636875747A2D6265726C696E2E64650D0A416E737072656368706172746E65723A2044722E20416C6578616E64657220446978"
								 "        A6 1E 131C68747470733a2f2f7777772e72656469726563742d746573742e6465"
								 "        A7 818B"
								 "            31 8188"
								 "                04 20 0617A64C37179860FEA3044A14052505FB4F8EF4D61F047B6A30130CD06718E6"
								 "                04 20 718FA21C2DF3A81BEF1B7A53360481C78C017F1EC7170266C2C4E9BB654B0E3B"
								 "                04 20 94B0AA7E8114F3E6DFCD52DA9F43E8B13CCB0589B8957E364728198FB4971AE6"
								 "                04 20 E85E1E8A78864E9246C86CF1C2A3810603EEEE75746C70CD51ACB86B5E2655D8"
								 "");

			QCOMPARE(certDescr->encode().toHex().toUpper(), QByteArray::fromHex(hexString).toHex().toUpper());

			CertificateDescription_free(certDescr);
		}


};

QTEST_GUILESS_MAIN(test_CertificateDescription)
#include "test_CertificateDescription.moc"
