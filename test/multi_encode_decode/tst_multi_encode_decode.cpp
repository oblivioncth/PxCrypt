// Qt Includes
#include <QtTest>

// Project Includes
#include <pxcrypt/codec/multi_encoder.h>
#include <pxcrypt/codec/multi_decoder.h>
#include <pxcrypt/stat.h>

// Test Includes
#include <pxcrypt_test_common.h>

// Test
class tst_encode_decode : public QObject
{
    Q_OBJECT

public:
    tst_encode_decode();

private slots:
    // Init
//    void initTestCase();
//    void cleanupTestCase();

    // Test cases

    void full_data_cycle_data();
    void full_data_cycle();
};

tst_encode_decode::tst_encode_decode() {}
//void tst_encode_decode::initTestCase() {}
//void tst_encode_decode::cleanupTestCase() {}

void tst_encode_decode::full_data_cycle_data()
{
    // Setup test table
    QTest::addColumn<QList<QImage>>("mediums");
    QTest::addColumn<QString>("tag");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QByteArray>("psk");
    QTest::addColumn<quint8>("bpc");
    QTest::addColumn<PxCrypt::Encoder::Encoding>("encoding");

    // Helper
    struct CycleTest
    {
        QString testName;
        qsizetype mediumCount;
        qsizetype payloadSize;
        QByteArray psk;
        quint8 bpc = 0;
        PxCrypt::Encoder::Encoding encoding;
    };

    auto addTestRow = [&](const CycleTest& t)
    {
        // The narrowing conversions here are not of concern

        // Create payload
        QRandomGenerator dRng(t.payloadSize);
        QByteArray payload(t.payloadSize, Qt::Uninitialized);
        std::generate(payload.begin(), payload.end(), [&dRng]{ return dRng.generate(); });

        // Create mediums purely randomly, really stress tests the lib
        auto rng = QRandomGenerator::global();
        QList<QImage> mediums(t.mediumCount);
        for(qsizetype i = 0; i < t.mediumCount; ++i)
        {
            QImage img(rng->bounded(100, 200), rng->bounded(50, 300), QImage::Format_ARGB32);
            QRandomGenerator::global()->fillRange(reinterpret_cast<quint32*>(img.bits()), img.sizeInBytes()/4);
            mediums[i] = img;
        }

        QTest::newRow(C_STR(t.testName)) << mediums << t.testName << payload << t.psk << t.bpc << t.encoding;
    };

    //### Populate test table rows with each case ################################

    // General test
    const CycleTest general{
        .testName = "General MultiEncode Test",
        .mediumCount = 10,
        .payloadSize = 15'000,
        .psk = "\xF9\xD3\xA2\x10"_ba,
        .encoding = PxCrypt::Encoder::Relative,
    };
    addTestRow(general);
}

void tst_encode_decode::full_data_cycle()
{
    // Fetch data from test table
    QFETCH(QList<QImage>, mediums);
    QFETCH(QString, tag);
    QFETCH(QByteArray, payload);
    QFETCH(QByteArray, psk);
    QFETCH(quint8, bpc);
    QFETCH(PxCrypt::Encoder::Encoding, encoding);

    // Encode
    PxCrypt::MultiEncoder enc;
    enc.setBpc(bpc);
    enc.setPresharedKey(psk);
    enc.setEncoding(encoding);
    enc.setTag(tag.toUtf8());

    QList<QImage> encoded;
    auto eErr = enc.encode(encoded, payload, mediums);
    QVERIFY2(!eErr, C_STR(eErr.errorString()));

    // Decode
    PxCrypt::MultiDecoder dec;
    dec.setPresharedKey(psk);

    QByteArray decoded;
    auto dErr = dec.decode(decoded, encoded, mediums);
    QVERIFY2(!dErr, C_STR(dErr.errorString()));

    // Compare
    QCOMPARE(decoded, payload);
    QCOMPARE(dec.tag(), tag);
}

QTEST_APPLESS_MAIN(tst_encode_decode)
#include "tst_multi_encode_decode.moc"
