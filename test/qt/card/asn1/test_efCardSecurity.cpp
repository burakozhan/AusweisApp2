/*!
 * \copyright Copyright (c) 2015-2018 Governikus GmbH & Co. KG, Germany
 */

#include "asn1/EFCardSecurity.h"
#include "TestFileHelper.h"

#include <QSharedPointer>
#include <QtTest>

using namespace governikus;

class test_efCardSecurity
	: public QObject
{
	Q_OBJECT

	private Q_SLOTS:
		void parseCrap()
		{
			QVERIFY(EFCardSecurity::fromHex("06092a864886f70d010702") == nullptr);
		}


		void noSignedDataOid()
		{
			QByteArray hexString("30 0F"
								 "           06 09 2A864886F70D01070F"
								 "           A0 02 0500");

			auto efCardSecurity = EFCardSecurity::fromHex(hexString);

			QVERIFY(efCardSecurity == nullptr);
		}


		void noSignedData()
		{
			QByteArray hexString("30 0F"
								 "           06 09 2A864886F70D010702"
								 "           A0 02 0500");

			auto efCardSecurity = EFCardSecurity::fromHex(hexString);

			QVERIFY(efCardSecurity == nullptr);
		}


		void brokenSignedData()
		{
			QByteArray hexString("30 18"
								 "           06 09 2A864886F70D010702"
								 "           A0 0B"
								 "               30 09"
								 "                   02 01 01"
								 "                   31 00"
								 "                   30 00"
								 "                   31 00");

			auto efCardSecurity = EFCardSecurity::fromHex(hexString);

			QVERIFY(efCardSecurity == nullptr);
		}


		void noSecurityObjectOid()
		{
			QByteArray hexString("30 820673"
								 "           06 09 2a864886f70d010702"
								 "           a0 820664"
								 "               30 820660"
								 "                   02 01 03"
								 "                   31 0f"
								 "                       30 0d"
								 "                           06 09 608648016503040201"
								 "                           05 00"
								 "                       30 820148"
								 "                           06 08 04007f000703020F"
								 "                           a0 82013a"
								 "                               04 820136"
								 "                                   31 820132 300d060804007f00070202020201023012060a04007f000702020302020201020201413012060a04007f0007020204020202010202010d3017060a04007f0007020205020330090201010201430101ff3017060a04007f0007020205020330090201010201440101003019060904007f000702020502300c060704007f0007010202010d301c060904007f000702020302300c060704007f0007010202010d020141302a060804007f0007020206161e687474703a2f2f6273692e62756e642e64652f6369662f6e70612e786d6c3062060904007f0007020201023052300c060704007f0007010202010d0342000498730cc691dde04358c2a6d71fa0919051211cf1987d1ed494c038d380121c72387637f7b62de03eafe0f54a2ce80ed0b7976e371cf6691160b2b1d7d6138a90020141a08203e4308203e030820367a003020102020120300a06082a8648ce3d0403033055310b3009060355040613024445310d300b060355040a0c0462756e64310c300a060355040b0c03627369310d300b0603550405130430303031311a301806035504030c115445535420637363612d6765726d616e79301e170d3132313031373039303133335a170d3233303431373233353935395a306d310b3009060355040613024445311d301b060355040a0c1442756e646573647275636b6572656920476d6248310d300b06035504051304303032313130302e06035504030c275445535420446f63756d656e74205369676e6572204964656e7469747920446f63756d656e7473308201333081ec06072a8648ce3d02013081e0020101302c06072a8648ce3d0101022100a9fb57dba1eea9bc3e660a909d838d726e3bf623d52620282013481d1f6e5377304404207d5a0975fc2c3057eef67530417affe7fb8055c126dc5c6ce94a4b44f330b5d9042026dc5c6ce94a4b44f330b5d9bbd77cbf958416295cf7e1ce6bccdc18ff8c07b60441048bd2aeb9cb7e57cb2c4b482ffc81b7afb9de27e1e3bd23c23a4453bd9ace3262547ef835c3dac4fd97f8461a14611dc9c27745132ded8e545c1d54c72f046997022100a9fb57dba1eea9bc3e660a909d838d718c397aa3b561a6f7901e0e82974856a70201010342000476dc598ae99ffad6bf0bc8962ad1146256324cb7a591983425c5731062f6d8bf192997ce6842e8205b3d3fbc1c04e180797d816cffc2b3886bdf23eb55c80f7ca38201323082012e301f0603551d2304183016801429f3f2cfa33b0b03b769d62c130dc0a01e79f98f301d0603551d0e041604148ca631a9bfe02418df8e2982d6abb438976d5484300e0603551d0f0101ff040403020780302b0603551d1004243022800f32303132313031373039303133335a810f32303133303631373233353935395a30160603551d20040f300d300b060904007f000703010101301d0603551d1104163014821262756e646573647275636b657265692e646530410603551d12043a30388118637363612d6765726d616e79406273692e62756e642e6465861c68747470733a2f2f7777772e6273692e62756e642e64652f6373636130350603551d1f042e302c302aa028a0268624687474703a2f2f7777772e6273692e62756e642e64652f746573745f637363615f63726c300a06082a8648ce3d04030303670030640230524026b7848be402679a742a82f8542e30188a39ce2d66b72034d959e1ceadd316c43a1d13a97953365450694fabbbee023038b1e256ec8e831b2dfbb9d7489aba4afc2ac9047a27f94f60a872a07ba3d4f639d637bbc1fb2cebf8830b5a57ec5f613182011430820110020101305a3055310b3009060355040613024445310d300b060355040a0c0462756e64310c300a060355040b0c03627369310d300b0603550405130430303031311a301806035504030c115445535420637363612d6765726d616e79020120300d06096086480165030402010500a04a301706092a864886f70d010903310a060804007f0007030201302f06092a864886f70d01090431220420224e458d73f90cb30a6b099931682b7cdf3c91ce8e5398c940081cb0f7934eef300c06082a8648ce3d04030205000446304402207f4666ff76cc2ecd3e044609d7a572663f1775a2c0f19315395cf895519ed5fd0220336c18314f55642718afe7c1116cfd235236bfb40f2f87155069a04b0bdfc077");

			auto efCardSecurity = EFCardSecurity::fromHex(hexString);

			QVERIFY(efCardSecurity == nullptr);
		}


		void noSecurityObject()
		{
			QByteArray hexString("30 820673"
								 "           06 09 2a864886f70d010702"
								 "           a0 820664"
								 "               30 820660"
								 "                   02 01 03"
								 "                   31 0f"
								 "                       30 0d"
								 "                           06 09 608648016503040201"
								 "                           05 00"
								 "                       30 820148"
								 "                           06 08 04007f0007030201"
								 "                           a0 82013a"
								 "                               04 820136"
								 "                                   000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");

			auto efCardSecurity = EFCardSecurity::fromHex(hexString);

			QVERIFY(efCardSecurity == nullptr);
		}


		void brokenSecurityInfos()
		{
			QByteArray hexString("30 820673"
								 "           06 09 2a864886f70d010702"
								 "           a0 820664"
								 "               30 820660"
								 "                   02 01 03"
								 "                   31 0f"
								 "                       30 0d"
								 "                           06 09 608648016503040201"
								 "                           05 00"
								 "                       30 820148"
								 "                           06 08 04007f0007030201"
								 "                           a0 82013a"
								 "                               04 820136"
								 "                                   31 820132 300d070804007f00070202020201023012060a04007f000702020302020201020201413012060a04007f0007020204020202010202010d3017060a04007f0007020205020330090201010201430101ff3017060a04007f0007020205020330090201010201440101003019060904007f000702020502300c060704007f0007010202010d301c060904007f000702020302300c060704007f0007010202010d020141302a060804007f0007020206161e687474703a2f2f6273692e62756e642e64652f6369662f6e70612e786d6c3062060904007f0007020201023052300c060704007f0007010202010d0342000498730cc691dde04358c2a6d71fa0919051211cf1987d1ed494c038d380121c72387637f7b62de03eafe0f54a2ce80ed0b7976e371cf6691160b2b1d7d6138a90020141a08203e4308203e030820367a003020102020120300a06082a8648ce3d0403033055310b3009060355040613024445310d300b060355040a0c0462756e64310c300a060355040b0c03627369310d300b0603550405130430303031311a301806035504030c115445535420637363612d6765726d616e79301e170d3132313031373039303133335a170d3233303431373233353935395a306d310b3009060355040613024445311d301b060355040a0c1442756e646573647275636b6572656920476d6248310d300b06035504051304303032313130302e06035504030c275445535420446f63756d656e74205369676e6572204964656e7469747920446f63756d656e7473308201333081ec06072a8648ce3d02013081e0020101302c06072a8648ce3d0101022100a9fb57dba1eea9bc3e660a909d838d726e3bf623d52620282013481d1f6e5377304404207d5a0975fc2c3057eef67530417affe7fb8055c126dc5c6ce94a4b44f330b5d9042026dc5c6ce94a4b44f330b5d9bbd77cbf958416295cf7e1ce6bccdc18ff8c07b60441048bd2aeb9cb7e57cb2c4b482ffc81b7afb9de27e1e3bd23c23a4453bd9ace3262547ef835c3dac4fd97f8461a14611dc9c27745132ded8e545c1d54c72f046997022100a9fb57dba1eea9bc3e660a909d838d718c397aa3b561a6f7901e0e82974856a70201010342000476dc598ae99ffad6bf0bc8962ad1146256324cb7a591983425c5731062f6d8bf192997ce6842e8205b3d3fbc1c04e180797d816cffc2b3886bdf23eb55c80f7ca38201323082012e301f0603551d2304183016801429f3f2cfa33b0b03b769d62c130dc0a01e79f98f301d0603551d0e041604148ca631a9bfe02418df8e2982d6abb438976d5484300e0603551d0f0101ff040403020780302b0603551d1004243022800f32303132313031373039303133335a810f32303133303631373233353935395a30160603551d20040f300d300b060904007f000703010101301d0603551d1104163014821262756e646573647275636b657265692e646530410603551d12043a30388118637363612d6765726d616e79406273692e62756e642e6465861c68747470733a2f2f7777772e6273692e62756e642e64652f6373636130350603551d1f042e302c302aa028a0268624687474703a2f2f7777772e6273692e62756e642e64652f746573745f637363615f63726c300a06082a8648ce3d04030303670030640230524026b7848be402679a742a82f8542e30188a39ce2d66b72034d959e1ceadd316c43a1d13a97953365450694fabbbee023038b1e256ec8e831b2dfbb9d7489aba4afc2ac9047a27f94f60a872a07ba3d4f639d637bbc1fb2cebf8830b5a57ec5f613182011430820110020101305a3055310b3009060355040613024445310d300b060355040a0c0462756e64310c300a060355040b0c03627369310d300b0603550405130430303031311a301806035504030c115445535420637363612d6765726d616e79020120300d06096086480165030402010500a04a301706092a864886f70d010903310a060804007f0007030201302f06092a864886f70d01090431220420224e458d73f90cb30a6b099931682b7cdf3c91ce8e5398c940081cb0f7934eef300c06082a8648ce3d04030205000446304402207f4666ff76cc2ecd3e044609d7a572663f1775a2c0f19315395cf895519ed5fd0220336c18314f55642718afe7c1116cfd235236bfb40f2f87155069a04b0bdfc077");

			auto efCardSecurity = EFCardSecurity::fromHex(hexString);

			QVERIFY(efCardSecurity == nullptr);
		}


		void parseEFCardSecurity()
		{
			QByteArray bytes = QByteArray::fromHex(TestFileHelper::readFile(":/card/efCardSecurity.hex"));

			auto efCardSecurity = EFCardSecurity::decode(bytes);

			QVERIFY(efCardSecurity != nullptr);
			QCOMPARE(efCardSecurity->getSecurityInfos()->getSecurityInfos().size(), 9);
		}


};

QTEST_GUILESS_MAIN(test_efCardSecurity)
#include "test_efCardSecurity.moc"
