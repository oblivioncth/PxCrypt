// Qt Includes
#include <QtTest>

// Project Includes
#include <pxcrypt/codec/standard_encoder.h>
#include <pxcrypt/codec/standard_decoder.h>
#include <pxcrypt/stat.h>

// Qx Includes
#include <qx/utility/qx-macros.h>

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
    QTest::addColumn<QImage>("medium");
    QTest::addColumn<QString>("tag");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QByteArray>("psk");
    QTest::addColumn<quint8>("bpc");
    QTest::addColumn<PxCrypt::Encoder::Encoding>("encoding");

    // Helper
    struct CycleTest
    {
        QString testName;
        QImage medium;
        qsizetype payloadSize;
        QByteArray psk;
        quint8 bpc;
        PxCrypt::Encoder::Encoding encoding;
    };

    auto addTestRow = [&](const CycleTest& t)
    {
        // The narrowing conversions here are not of concern

        // Create payload
        QRandomGenerator rng(t.payloadSize);
        QByteArray payload(t.payloadSize, Qt::Uninitialized);
        std::generate(payload.begin(), payload.end(), [&rng]{ return rng.generate(); });

        QTest::newRow(C_STR(t.testName)) << t.medium << t.testName << payload << t.psk << t.bpc << t.encoding;
    };

    //### Populate test table rows with each case ################################

    QImage realWorldImage(":/data/real_world_image.jpg");
    QVERIFY2(!realWorldImage.isNull(), "failed to load real world image.");

    //-Relative BPC Tests---------------------------------------------------------
    const CycleTest relativeBPC{
        .testName = "Relative BPC - ",
        .medium = realWorldImage,
        .payloadSize = 1000,
        .psk = QBAL("\x49\xAE\xC4\xDE"),
        .encoding = PxCrypt::Encoder::Relative
    };

    for(quint8 i = 0; i < 8; i++)
    {
        CycleTest t = relativeBPC;
        t.testName += QString::number(i);
        t.bpc = i;
        addTestRow(t);
    }

    // Absolute BPC Tests
    const CycleTest absoluteBPC{
        .testName = "Absolute BPC - ",
        .medium = realWorldImage,
        .payloadSize = 1000,
        .psk = QBAL("\x20\x4F\xA9\xCA"),
        .encoding = PxCrypt::Encoder::Absolute
    };

    for(quint8 i = 0; i < 8; i++)
    {
        CycleTest t = absoluteBPC;
        t.testName += QString::number(i);
        t.bpc = i;
        addTestRow(t);
    }

    /* Perfect fit test ---------------------------------------------------------------------
     *
     * This test stores data that fits perfectly (i.e. 100% bit capacity used) within an image
     * as densely as possible. Uses 4 BPC because then the byte size of the data just needs to
     * be a multiple of 3 to fit perfectly
     *
     * Bytes = magic + renditionId + tagLength + tag + checksum + payloadSize + payload
     *       = 3 + 2 + 2 + 16 + 4 + 4 + 2
     *       = 33
     *
     * Bits = bytes * 8
     *      = 264
     *
     * Pixels = ceil(bits / (bpc * 3)) + metaPixels
     *        = ceil(264 / (4 * 3)) + 2
     *        = ceil(22) + 2
     *        = 22 + 2
     *        = 24
     *
     * Size: 8 x 3
     *
     * Would be great if the associated types are eventually constexpr compatible so that the dims
     * can be calculated automatically.
     */

    quint8 pfBpc = 4;
    QString pfTag = "Perfect Fit Test"; // 16 char
    qsizetype pfPayload = 2;
    QImage pfMedium(8, 3, QImage::Format_ARGB32);
    pfMedium.fill(Qt::gray);

    PxCrypt::Stat::Capacity pfCapacity = PxCrypt::Stat(pfMedium).capacity(pfBpc);
    qsizetype max = PxCrypt::StandardEncoder::calculateMaximumPayload(pfMedium.size(), pfTag.size(), pfBpc);
    QVERIFY2(pfCapacity.leftoverBits == 0 && pfPayload == max, "Perfect fit test payload/image size needs adjustment");

    CycleTest perfectFitTest{
        .testName = pfTag,
        .medium = pfMedium,
        .payloadSize = pfPayload,
        .psk = QBAL("\38\xDF\xE1\x4F"),
        .bpc = pfBpc,
        .encoding = PxCrypt::Encoder::Absolute
    };

    addTestRow(perfectFitTest);

    /* Maximum density test (max capacity at smallest possible size) ----------------------------
     *
     * This test stores 3 pixels of payload data at 7 BPC within the smallest possible image.
     * This is the largest payload possible that can be stored within such an image.
     *
     * Bytes = magic + renditionId + tagLength + tag + checksum + payloadSize + payload
     *       = 3 + 2 + 2 + 16 + 4 + 4 + 3
     *       = 34
     *
     * Bits = bytes * 8
     *      = 272
     *
     * Pixels = ceil(bits / (bpc * 3)) + metaPixels
     *        = ceil(272 / (7 * 3)) + 2
     *        = ceil(12.9523) + 2
     *        = 13 + 2
     *        = 15
     *
     * Size: 5 x 3
     */
    quint8 denseBpc = 7;
    QString denseTag = "Max Density Test"; // 16 char
    qsizetype densePayload = 3;
    QImage denseMedium(5, 3, QImage::Format_ARGB32);
    denseMedium.fill(Qt::gray);

    qsizetype maxPayload = PxCrypt::StandardEncoder::calculateMaximumPayload(denseMedium.size(), denseTag.size(), denseBpc);
    QVERIFY2(maxPayload == densePayload, "Max density test payload/image size needs adjustment");

    CycleTest densityTest{
        .testName = denseTag,
        .medium = denseMedium,
        .payloadSize = densePayload,
        .psk = QBAL("\x25\xE6\x1A\x83"),
        .bpc = denseBpc,
        .encoding = PxCrypt::Encoder::Absolute
    };

    addTestRow(densityTest);

    //-"Real world" Max capacity tests------------------------------------------------------

    // Max capacity tests
    QString mttR = "Maximum Capacity Test - Relative";
    qsizetype mpsR = PxCrypt::StandardEncoder::calculateMaximumPayload(realWorldImage.size(), mttR.size(), 7);
    QString mttA = "Maximum Capacity Test - Absolute";
    qsizetype mpsA = PxCrypt::StandardEncoder::calculateMaximumPayload(realWorldImage.size(), mttA.size(), 7);

    CycleTest maxTestR{
        .testName = mttR,
        .medium = realWorldImage,
        .payloadSize = mpsR,
        .psk = QBAL("\x88\x39\xAB\xF7"),
        .bpc = 7,
        .encoding = PxCrypt::Encoder::Relative
    };

    CycleTest maxTestA{
        .testName = mttA,
        .medium = realWorldImage,
        .payloadSize = mpsA,
        .psk = QBAL("\x69\x3D\xE4\xB0"),
        .bpc = 7,
        .encoding = PxCrypt::Encoder::Absolute
    };

    addTestRow(maxTestR);
    addTestRow(maxTestA);

    //-Non-native format test----------------------------------------------------------------
    QImage nn(200, 200, QImage::Format_Indexed8);
    nn.fill(Qt::gray);

    CycleTest nonNativeTest{
        .testName = "Non-native format test",
        .medium = nn,
        .payloadSize = 1000,
        .psk = QBAL("\x50\x60\x70\x80"),
        .bpc = 1,
        .encoding = PxCrypt::Encoder::Relative
    };

    addTestRow(nonNativeTest);
}

void tst_encode_decode::full_data_cycle()
{
    // Fetch data from test table
    QFETCH(QImage, medium);
    QFETCH(QString, tag);
    QFETCH(QByteArray, payload);
    QFETCH(QByteArray, psk);
    QFETCH(quint8, bpc);
    QFETCH(PxCrypt::Encoder::Encoding, encoding);

    // Encode
    PxCrypt::StandardEncoder enc;
    enc.setBpc(bpc);
    enc.setPresharedKey(psk);
    enc.setEncoding(encoding);
    enc.setTag(tag.toUtf8());

    QImage encoded;
    PxCrypt::StandardEncoder::Error eErr = enc.encode(encoded, payload, medium);
    QVERIFY2(!eErr, C_STR(eErr.errorString()));

    // Decode
    PxCrypt::StandardDecoder dec;
    dec.setPresharedKey(psk);

    QByteArray decoded;
    PxCrypt::StandardDecoder::Error dErr = dec.decode(decoded, encoded, medium);
    QVERIFY2(!dErr, C_STR(dErr.errorString()));

    // Compare
    QCOMPARE(decoded, payload);
    QCOMPARE(dec.tag(), tag);
}

QTEST_APPLESS_MAIN(tst_encode_decode)
#include "tst_encode_decode.moc"
