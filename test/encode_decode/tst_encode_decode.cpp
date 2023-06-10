// Qt Includes
#include <QtTest>

// Project Includes
#include <pxcrypt/encode.h>
#include <pxcrypt/decode.h>
#include <pxcrypt/_internal.h>

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
    QTest::addColumn<PxCrypt::EncType>("encType");

    // Helper
    struct CycleTest
    {
        QString testName;
        QImage medium;
        qsizetype payloadSize;
        QByteArray psk;
        quint8 bpc;
        PxCrypt::EncType encType;
    };

    auto addTestRow = [&](const CycleTest& t)
    {
        // The narrowing conversions here are not of concern

        // Create payload
        QRandomGenerator rng(t.payloadSize);
        QByteArray payload(t.payloadSize, Qt::Uninitialized);
        std::generate(payload.begin(), payload.end(), [&rng]{ return rng.generate(); });

        QTest::newRow(C_STR(t.testName)) << t.medium << t.testName << payload << t.psk << t.bpc << t.encType;
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
        .encType = PxCrypt::EncType::Relative
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
        .encType = PxCrypt::EncType::Absolute
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
     * as densely as possible. Uses 2 BPC because then the byte size of the data just needs to
     * be a multiple of 3 to fit perfectly
     *
     * Bytes = header + payloadSize + tagSize
     *       = (3 + 4 + 2 + 4) + 4 + 16
     *       = (13) + 20
     *       = 33
     *
     * Bits = bytes * 8
     *      = 264
     *
     * Pixels = ceil(bits / (bpc * 3)) + metaPixels
     *        = ceil(256 / (4 * 3)) + 2
     *        = ceil(22) + 2
     *        = 22 + 2
     *        = 24
     *
     * Size: 8 x 3
     */

    QString pfTag = "Perfect Fit Test"; // 16 char
    qsizetype pfPayload = 4;
    QImage pfMedium(8, 3, QImage::Format_ARGB32);
    pfMedium.fill(Qt::gray);

    QVERIFY2(_PxCrypt::calculateMaximumPayloadBits(pfMedium.size(), pfTag.size(), 4) == pfPayload * 8, "Perfect fit test payload/image size needs adjustment");
    CycleTest perfectFitTest{
        .testName = pfTag,
        .medium = pfMedium,
        .payloadSize = pfPayload,
        .psk = QBAL("\38\xDF\xE1\x4F"),
        .bpc = 4,
        .encType = PxCrypt::EncType::Absolute
    };

    addTestRow(perfectFitTest);

    /* Maximum density test (max capacity at smallest possible size) ----------------------------
     *
     * This test stores 1 pixel of payload data at 7 BPC within the smallest possible image
     *
     * Bytes = header + payloadSize + tagSize
     *       = (3 + 4 + 2 + 4) + 2 + 16
     *       = (13) + 18
     *       = 31
     *
     * Bits = bytes * 8
     *      = 248
     *
     * Pixels = ceil(bits / (bpc * 3)) + metaPixels
     *        = ceil(248 / (7 * 3)) + 2
     *        = ceil(11.8095) + 2
     *        = 12 + 2
     *        = 14
     *
     * Size: 7 x 2
     */
    QString denseTag = "Max Density Test"; // 16 char
    qsizetype densePayload = 2;
    QImage denseMedium(7, 2, QImage::Format_ARGB32);
    denseMedium.fill(Qt::gray);

    QVERIFY2(PxCrypt::calculateMaximumStorage(denseMedium.size(), denseTag.size(), 7) == densePayload, "Max density test payload/image size needs adjustment");
    CycleTest densityTest{
        .testName = denseTag,
        .medium = denseMedium,
        .payloadSize = densePayload,
        .psk = QBAL("\x25\xE6\x1A\x83"),
        .bpc = 7,
        .encType = PxCrypt::EncType::Absolute
    };

    addTestRow(densityTest);

    //-"Real world" Max capacity tests------------------------------------------------------

    // Max capacity tests
    QString mttR = "Maximum Capacity Test - Relative";
    qsizetype mpsR = PxCrypt::calculateMaximumStorage(realWorldImage.size(), mttR.size(), 7);
    QString mttA = "Maximum Capacity Test - Absolute";
    qsizetype mpsA = PxCrypt::calculateMaximumStorage(realWorldImage.size(), mttA.size(), 7);

    CycleTest maxTestR{
        .testName = mttR,
        .medium = realWorldImage,
        .payloadSize = mpsR,
        .psk = QBAL("\x88\x39\xAB\xF7"),
        .bpc = 7,
        .encType = PxCrypt::EncType::Relative
    };

    CycleTest maxTestA{
        .testName = mttA,
        .medium = realWorldImage,
        .payloadSize = mpsA,
        .psk = QBAL("\x69\x3D\xE4\xB0"),
        .bpc = 7,
        .encType = PxCrypt::EncType::Absolute
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
        .encType = PxCrypt::EncType::Relative
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
    QFETCH(PxCrypt::EncType, encType);

    // Encode
    PxCrypt::EncodeSettings es{
        .bpc = bpc,
        .psk = psk,
        .type = encType
    };

    QImage encoded;
    Qx::GenericError ee = PxCrypt::encode(encoded, medium, tag, payload, es);
    QVERIFY2(!ee.isValid(), C_STR(ee.secondaryInfo()));

    // Decode
    QByteArray decoded;
    QString tagDecoded;
    Qx::GenericError de = PxCrypt::decode(decoded, tagDecoded, encoded, psk, medium);
    QVERIFY2(!de.isValid(), C_STR(de.secondaryInfo()));

    // Compare
    QCOMPARE(payload, decoded);
    QCOMPARE(tag, tagDecoded);
}

QTEST_APPLESS_MAIN(tst_encode_decode)
#include "tst_encode_decode.moc"
