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

    //-Populate test table rows with each case-----------------------------

    QImage realWorldImage(":/data/real_world_image.jpg");
    QVERIFY2(!realWorldImage.isNull(), "failed to load real world image.");

    // Relative BPC Tests
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

    // Non-native format test
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
    PxCrypt::DecodeSettings ds{
        .psk = psk,
        .type = encType
    };

    QByteArray decoded;
    QString tagDecoded;
    Qx::GenericError de = PxCrypt::decode(decoded, tagDecoded, encoded, ds, medium);
    QVERIFY2(!de.isValid(), C_STR(de.secondaryInfo()));

    // Compare
    QCOMPARE(payload, decoded);
    QCOMPARE(tag, tagDecoded);
}

QTEST_APPLESS_MAIN(tst_encode_decode)
#include "tst_encode_decode.moc"
