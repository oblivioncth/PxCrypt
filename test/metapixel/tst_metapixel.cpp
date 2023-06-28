// Qt Includes
#include <QtTest>

// Project Includes
#include <pxcrypt/encoder.h>
#include <pxcrypt/decoder.h>

// Qx Includes
#include <qx/utility/qx-macros.h>

// Test Includes
#include <pxcrypt_test_common.h>

// Test
class tst_metapixel : public QObject
{
    Q_OBJECT

private:
    // Testing
    QString mPayloadName;
    QByteArray mPayload;

public:
    tst_metapixel();

private slots:
    // Init
//    void initTestCase();
//    void cleanupTestCase();

    // Test cases
    void invalid_meta_data();
    void invalid_meta();

};

tst_metapixel::tst_metapixel() {}
//void tst_encode_decode::initTestCase() {}
//void tst_encode_decode::cleanupTestCase() {}

void tst_metapixel::invalid_meta_data()
{
    // Ensure all images from various platforms can be decoded on this platform
    QTest::addColumn<QString>("encodedPath");
    QTest::addColumn<QString>("expectedError");

    // Add test rows
    QString metaError = "The provided image is not encoded.";
    QDir data(":/data");
    QTest::newRow("invalid_bpc") << data.absoluteFilePath("invalid_bpc.png") << metaError;
    QTest::newRow("invalid_encoding") << data.absoluteFilePath("invalid_encoding.png") << metaError;

    // NOTE: Current invalid encoding test uses value of 7, if that value ends up occupied then this
    // needs to change
}

void tst_metapixel::invalid_meta()
{
    // Fetch data from test table
    QFETCH(QString, encodedPath);
    QFETCH(QString, expectedError);

    // Load image
    QImage encoded(encodedPath);
    QVERIFY(!encoded.isNull());

    // Decode
    PxCrypt::Decoder dec;

    dec.decode(encoded);
    QVERIFY2(dec.hasError(), "Image is not purposely flawed correctly");
    QCOMPARE(dec.error().secondaryInfo(), expectedError);
}

QTEST_APPLESS_MAIN(tst_metapixel)
#include "tst_metapixel.moc"
