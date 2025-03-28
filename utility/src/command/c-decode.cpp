// Unit Includes
#include "c-decode.h"

// Qt Includes
#include <QImageReader>
#include <QImageWriter>

// Qx Includes
#include <qx/io/qx-common-io.h>

// Project Includes
#include "pxcrypt/codec/standard_decoder.h"
#include "pxcrypt/codec/multi_decoder.h"
#include "utility.h"

//===============================================================================================================
// CDecodeError
//===============================================================================================================

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
CDecodeError::CDecodeError() :
    mType(NoError)
{}

//Private:
CDecodeError::CDecodeError(Type type, const QString& gen) :
    mType(type),
    mGeneral(gen)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint32 CDecodeError::deriveValue() const { return static_cast<quint32>(mType); }
QString CDecodeError::derivePrimary() const { return mGeneral; }
QString CDecodeError::deriveSecondary() const { return mSpecific; }
QString CDecodeError::deriveDetails() const { return mDetails; }

CDecodeError CDecodeError::wSpecific(const QString& spec, const QString& det) const
{
    CDecodeError s = *this;
    s.mSpecific = spec;
    s.mDetails = det;
    return s;
}

//Public:
bool CDecodeError::isValid() const { return mType != NoError; }
CDecodeError::Type CDecodeError::type() const { return mType; }
QString CDecodeError::errorString() const { return mGeneral + " " + mSpecific; }

//===============================================================================================================
// CDecode
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Public:
CDecode::CDecode(Core& coreRef) : Command(coreRef)
{}

//-Instance Functions-------------------------------------------------------------
//Private:
CDecodeError CDecode::loadMultiImages(QList<QImage>& images, const QStringList& paths, const CDecodeError& baseError)
{
    images.resize(paths.size());

    QImageReader imgReader;
    for(qsizetype i = 0; i < paths.size(); ++i)
    {
        auto& ip = paths.at(i);
        QImage& im = images[i];
        imgReader.setFileName(ip);
        if(!imgReader.read(&im))
            return baseError.wSpecific(imgReader.errorString(), ip);
    }

    return {};
}

Qx::Error CDecode::decodeSingleImage(QByteArray& decoded, QString& tag, const QString& encodedPath, const std::optional<QString>& mediumPath, QByteArrayView psk)
{
    QImageReader imgReader;

    // Load medium if provided
    QImage aMedium;
    if(mediumPath)
    {
        QString medPath = mediumPath.value();
        imgReader.setFileName(medPath);
        if(!imgReader.read(&aMedium))
            return ERR_MEDIUM_READ_FAILED.wSpecific(imgReader.errorString());
    }

    // Load encoded image
    QImage aEncoded;
    imgReader.setFileName(encodedPath);
    if(!imgReader.read(&aEncoded))
        return ERR_INPUT_READ_FAILED.wSpecific(imgReader.errorString());

    // Decode
    PxCrypt::StandardDecoder decoder;
    decoder.setPresharedKey(psk.toByteArray());

    mCore.printMessage(NAME, MSG_DECODING);
    if(auto err = decoder.decode(decoded, aEncoded, aMedium))
        return err;

    tag = decoder.tag();
    return {};
}

Qx::Error CDecode::decodeMultipleImages(QByteArray& decoded, QString& tag,  const QDir& encodedDir, const std::optional<QDir>& mediumDir, QByteArrayView psk)
{
    // TODO: Don't even try to load lossy image formats

    // Get list of encoded images
    QStringList encodedPaths;
    if(auto rp = Qx::dirContentList(encodedPaths, encodedDir, mCore.imageFormatFilter(), QDir::NoFilter, QDirIterator::Subdirectories); rp.isFailure())
        return rp;
    else if(encodedPaths.isEmpty())
        return ERR_INPUT_READ_FAILED.wSpecific(u"No encoded images were found in the provided directory."_s);

    // Get list of medium images, if applicable
    QStringList mediumPaths;
    if(mediumDir)
    {
        if(auto rp = Qx::dirContentList(mediumPaths, mediumDir.value(), mCore.imageFormatFilter(), QDir::NoFilter, QDirIterator::Subdirectories); rp.isFailure())
            return rp;
        else if(mediumPaths.isEmpty())
            return ERR_INPUT_READ_FAILED.wSpecific(u"No medium images were found in the provided directory."_s);
        else if(encodedPaths.count() != mediumPaths.count())
            return ERR_MEDIUM_COUNT_MISMATCH.wSpecific(u"%1 encoded, %2 mediums"_s.arg(encodedPaths.count()).arg(mediumPaths.count()));
    }

    mCore.printMessage(NAME, MSG_MULTI_DECODE_COUNT.arg(encodedPaths.count()));

    // Load encoded images
    QList<QImage> encoded;
    if(auto err = loadMultiImages(encoded, encodedPaths, ERR_INPUT_READ_FAILED))
        return err;

    // Load mediums, if applicable
    QList<QImage> mediums;
    if(!mediumPaths.isEmpty())
    {
        if(auto err = loadMultiImages(mediums, mediumPaths, ERR_MEDIUM_READ_FAILED))
            return err;
    }

    // Decode
    PxCrypt::MultiDecoder decoder;
    decoder.setPresharedKey(psk.toByteArray());

    mCore.printMessage(NAME, MSG_DECODING);
    if(auto err = decoder.decode(decoded, encoded, mediums))
        return err;

    tag = decoder.tag();
    return {};
}

//Protected:
const QList<const QCommandLineOption*> CDecode::options() { return Command::options() + CL_OPTIONS_SPECIFIC; }
const QSet<const QCommandLineOption*> CDecode::requiredOptions() { return CL_OPTIONS_REQUIRED; }
const QString CDecode::name() { return NAME; }

//Public:
Qx::Error CDecode::perform()
{
    //-Preparation---------------------------------------

    // Get key
    QByteArray aKey = mParser.value(CL_OPTION_KEY).toUtf8();

    // Get input info
    QFileInfo encodedInfo(mParser.value(CL_OPTION_INPUT));

    // Ensure input exists
    if(!encodedInfo.exists())
    {
        CDecodeError err = ERR_INPUT_DOES_NOT_EXIST;
        mCore.printError(NAME, err);
        return err;
    }

    // Examine medium input
    QString medPath = mParser.isSet(CL_OPTION_MEDIUM) ? mParser.value(CL_OPTION_MEDIUM) : QString();
    if(!medPath.isNull())
    {
        QFileInfo medPathInfo(medPath);
        if(!medPathInfo.exists())
        {
            CDecodeError err = ERR_MEDIUM_DOES_NOT_EXIST;
            mCore.printError(NAME, err);
            return err;
        }
        else if(medPathInfo.isDir() ^ encodedInfo.isDir())
        {
            CDecodeError err = ERR_MEDIUM_TYPE_MISMATCH;
            mCore.printError(NAME, err);
            return err;
        }
    }

    //-Decoding---------------------------------------
    // TODO: Simplify argument passing
    QByteArray decoded;
    QString tag;
    Qx::Error jobError;
    if(encodedInfo.isDir())
    {
        std::optional<QDir> mediums = medPath.isNull() ? std::nullopt : std::optional<QDir>(QDir(medPath));
        jobError = decodeMultipleImages(decoded, tag, encodedInfo.absoluteFilePath(), mediums, aKey);
    }
    else
    {
        std::optional<QString> medium = medPath.isNull() ? std::nullopt : std::optional<QString>(medPath);
        jobError = decodeSingleImage(decoded, tag, encodedInfo.absoluteFilePath(), medium, aKey);
    }

    if(jobError)
    {
        mCore.printError(NAME, jobError);
        return jobError;
    }

    mCore.printMessage(NAME, MSG_PAYLOAD_SIZE.arg(Utility::dataStr(decoded.size())));
    mCore.printMessage(NAME, MSG_TAG.arg(tag));

    // Write decoded data
    QDir outputDir(mParser.isSet(CL_OPTION_OUTPUT) ? mParser.value(CL_OPTION_OUTPUT) : encodedInfo.absoluteDir());
    QFile outputFile(outputDir.absoluteFilePath(tag));

    Qx::IoOpReport wr = Qx::writeBytesToFile(outputFile, decoded, Qx::WriteMode::Truncate, 0, Qx::WriteOption::NewOnly | Qx::WriteOption::CreatePath);
    if(wr.isFailure())
    {
        mCore.printError(NAME, wr);
        return wr;
    }
    mCore.printMessage(NAME, MSG_DATA_SAVED.arg(outputFile.fileName()));

    return CDecodeError();
}
