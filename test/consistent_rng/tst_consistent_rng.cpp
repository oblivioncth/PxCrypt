// Qt Includes
#include <QtTest>

// Project Includes
#include <pxcrypt/encode.h>
#include <pxcrypt/decode.h>

// Qx Includes
#include <qx/utility/qx-macros.h>

// Test Includes
#include <pxcrypt_test_common.h>

// Test
class tst_consistent_rng : public QObject
{
    Q_OBJECT

private:
    // Testing
    QString mPayloadName;
    QByteArray mPayload;

public:
    tst_consistent_rng();

private slots:
    // Init
//    void initTestCase();
//    void cleanupTestCase();

    // Test cases
    void multi_platform_decode_data();
    void multi_platform_decode();

};

tst_consistent_rng::tst_consistent_rng()
{
    // Load payload
    mPayloadName = QSL("payload.txt");
    QFile payloadFile(":/data/" + mPayloadName);
    payloadFile.open(QIODevice::ReadOnly);
    mPayload = payloadFile.readAll();
    QVERIFY(!mPayload.isEmpty());
    payloadFile.close();
}
//void tst_encode_decode::initTestCase() {}
//void tst_encode_decode::cleanupTestCase() {}

void tst_consistent_rng::multi_platform_decode_data()
{
    // Ensure all images from various platforms can be decoded on this platform
    QTest::addColumn<QString>("encodedPath");

    QDir data(":/data");
    QFileInfoList encodedImageInfo = data.entryInfoList({"*.png"}, QDir::NoFilter, QDir::Name);

    for(const QFileInfo& info : encodedImageInfo)
        QTest::newRow(C_STR(info.baseName())) << info.absoluteFilePath();
}

void tst_consistent_rng::multi_platform_decode()
{
    // Fetch data from test table
    QFETCH(QString, encodedPath);

    // Load image
    QImage encoded(encodedPath);
    QVERIFY(!encoded.isNull());

    // Decode
    QByteArray decoded;
    QString tagDecoded;

    Qx::GenericError de = PxCrypt::decode(decoded, tagDecoded, encoded, PxCrypt::DecodeSettings());
    QVERIFY2(!de.isValid(), C_STR(de.secondaryInfo()));

    // Compare
    QCOMPARE(mPayload, decoded);
    QCOMPARE(mPayloadName, tagDecoded);
}

QTEST_APPLESS_MAIN(tst_consistent_rng)
#include "tst_consistent_rng.moc"
