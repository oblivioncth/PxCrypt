// Qt Includes
#include <QtTest>

// Project Includes
#include <pxcrypt/codec/standard_encoder.h>
#include <pxcrypt/codec/standard_decoder.h>

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
    QTest::addColumn<PxCrypt::StandardDecoder::Error::Type>("expectedError");

    // Add test rows
    QDir data(":/data");
    QTest::newRow("invalid_bpc") << data.absoluteFilePath("invalid_bpc.png") << PxCrypt::StandardDecoder::Error::InvalidMeta;
    QTest::newRow("invalid_encoding") << data.absoluteFilePath("invalid_encoding.png") << PxCrypt::StandardDecoder::Error::InvalidMeta;

    // NOTE: Current invalid encoding test uses value of 7, if that value ends up occupied then this
    // needs to change
}

void tst_metapixel::invalid_meta()
{
    // Fetch data from test table
    QFETCH(QString, encodedPath);
    QFETCH(PxCrypt::StandardDecoder::Error::Type, expectedError);

    // Load image
    QImage encoded(encodedPath);
    QVERIFY(!encoded.isNull());

    // Decode
    PxCrypt::StandardDecoder dec;

    QByteArray dummy;
    PxCrypt::StandardDecoder::Error err = dec.decode(dummy, encoded);
    QVERIFY2(err, "Image is not purposely flawed correctly");
    QCOMPARE(err.type(), expectedError);
}

QTEST_APPLESS_MAIN(tst_metapixel)
#include "tst_metapixel.moc"
