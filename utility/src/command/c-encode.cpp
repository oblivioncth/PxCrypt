// Unit Includes
#include "c-encode.h"

// Qt Includes
#include <QImageReader>
#include <QImageWriter>

// Qx Includes
#include <qx/io/qx-common-io.h>

// Project Includes
#include "pxcrypt/codec/standard_encoder.h"
#include "pxcrypt/codec/multi_encoder.h"
#include "utility.h"

//===============================================================================================================
// CEncodeError
//===============================================================================================================

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
CEncodeError::CEncodeError() :
    mType(NoError)
{}

//Private:
CEncodeError::CEncodeError(Type type, const QString& gen) :
    mType(type),
    mGeneral(gen)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint32 CEncodeError::deriveValue() const { return static_cast<quint32>(mType); }
QString CEncodeError::derivePrimary() const { return mGeneral; }
QString CEncodeError::deriveSecondary() const { return mSpecific; }
QString CEncodeError::deriveDetails() const { return mDetails; }

CEncodeError CEncodeError::wSpecific(const QString& spec, const QString& det) const
{
    CEncodeError s = *this;
    s.mSpecific = spec;
    s.mDetails = det;
    return s;
}

//Public:
bool CEncodeError::isValid() const { return mType != NoError; }
CEncodeError::Type CEncodeError::type() const { return mType; }
QString CEncodeError::errorString() const { return mGeneral + " " + mSpecific; }

//===============================================================================================================
// CEncode
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Public:
CEncode::CEncode(Core& coreRef) : Command(coreRef)
{}

//-Instance Functions-------------------------------------------------------------
//Private:
Qx::Error CEncode::encodeSingleImage(const Job& job)
{
    // Load medium
    QImageReader imgReader(job.mediumInfo.absoluteFilePath());
    QImage aMedium;
    if(!imgReader.read(&aMedium))
        return ERR_MEDIUM_READ_FAILED.wSpecific(imgReader.errorString());

    // Print medium size
    QSize mediumSize = aMedium.size();
    mCore.printMessage(NAME, MSG_SINGLE_MEDIUM_DIM.arg(mediumSize.width()).arg(mediumSize.height()));

    // Encode
    PxCrypt::StandardEncoder encoder;
    encoder.setBpc(job.bpc);
    encoder.setPresharedKey(job.psk.toByteArray());
    encoder.setEncoding(job.encoding);
    encoder.setTag(job.tag.toUtf8());

    mCore.printMessage(NAME, MSG_START_ENCODING);
    QImage encoded;
    if(auto err = encoder.encode(encoded, job.payload, aMedium))
        return err;

    // Print true density if auto was used
    if(job.bpc == 0)
        mCore.printMessage(NAME, MSG_SINGLE_ACTUAL_BPC.arg(encoder.bpc()));

    // Write encoded image
    QString outputPath;
    if(mParser.isSet(CL_OPTION_OUTPUT))
        outputPath = mParser.value(CL_OPTION_OUTPUT);
    else
    {
        QDir dir = job.inputInfo.absoluteDir();
        QString basename = job.inputInfo.baseName();
        outputPath = dir.absoluteFilePath(basename + "_enc." + OUTPUT_EXT);
    }

    if(QFile::exists(outputPath))
        return ERR_OUTPUT_WRITE_FAILED.wSpecific(u"The file already exists."_s);

    QImageWriter imgWriter(outputPath);
    if(!imgWriter.write(encoded))
        return ERR_OUTPUT_WRITE_FAILED.wSpecific(imgWriter.errorString());

    mCore.printMessage(NAME, MSG_SINGLE_IMAGE_SAVED.arg(outputPath));
    return {};
}

Qx::Error CEncode::encodeMultipleImages(const Job& job)
{
    // Get list of mediums
    QDir mediumDir(job.mediumInfo.absoluteFilePath());
    QStringList mediumPaths;
    if(auto rp = Qx::dirContentList(mediumPaths, mediumDir, mCore.imageFormatFilter(), QDir::NoFilter, QDirIterator::Subdirectories); rp.isFailure())
        return rp;
    else if(mediumPaths.isEmpty())
        return ERR_MEDIUM_READ_FAILED.wSpecific(u"No images were found in the provided directory."_s);

    mCore.printMessage(NAME, MSG_MULTI_IMAGE_COUNT.arg(mediumPaths.count()));

    /* Load all mediums
     *
     * TODO: We could reduce memory usage here by introducing an image wraper type (i.e. InputImage) or something
     * that the lib takes. It would have a ctor for a QFile or QString as a file path, and one for QImage. The
     * latter ctor would allow the library to work as is, where images are pre-loaded, but the former would allow
     * the image to be loaded on demand by the encoder from disk, so there would only be as many images in memory
     * at once as there are threads.
     */
    QImageReader imgReader;
    QList<QImage> mediums(mediumPaths.size());
    for(qsizetype i = 0; i < mediums.size(); ++i)
    {
        auto& mp = mediumPaths.at(i);
        QImage& m = mediums[i];
        imgReader.setFileName(mp);
        if(!imgReader.read(&m))
            return ERR_MEDIUM_READ_FAILED.wSpecific(imgReader.errorString(), mp);
    }

    // Encode
    PxCrypt::MultiEncoder encoder;
    encoder.setBpc(job.bpc);
    encoder.setPresharedKey(job.psk.toByteArray());
    encoder.setEncoding(job.encoding);
    encoder.setTag(job.tag.toUtf8());

    mCore.printMessage(NAME, MSG_START_ENCODING);
    QList<QImage> encoded;
    if(auto err = encoder.encode(encoded, job.payload, mediums))
        return err;

    // Print approximate density if auto was used
    if(job.bpc == 0)
        mCore.printMessage(NAME, MSG_MULTI_APROX_BPC.arg(encoder.bpc()));

    // Setup root output exists
    QDir outputDir(mParser.isSet(CL_OPTION_OUTPUT) ?
        mParser.value(CL_OPTION_OUTPUT) :
        mediumDir.absolutePath() + "_enc"
    );
    if(!outputDir.mkpath(u"."_s))
        return ERR_OUTPUT_WRITE_FAILED.wSpecific(u"Failed to create root path."_s);

    // Write encoded images
    QImageWriter imgWriter;
    for(qsizetype i = 0; i < encoded.size(); ++i)
    {
        auto& mp = mediumPaths.at(i);
        const QString encodedPath = outputDir.absoluteFilePath(mediumDir.relativeFilePath(mp));
        auto& enc = encoded.at(i);
        imgWriter.setFileName(encodedPath);
        if(!imgWriter.write(enc))
            return ERR_OUTPUT_WRITE_FAILED.wSpecific(imgWriter.errorString(), encodedPath);
    }

    mCore.printMessage(NAME, MSG_MULTI_IMAGE_SAVED.arg(outputDir.absolutePath()));
    return {};
}

//Protected:
const QList<const QCommandLineOption*> CEncode::options() { return Command::options() + CL_OPTIONS_SPECIFIC; }
const QSet<const QCommandLineOption*> CEncode::requiredOptions() { return CL_OPTIONS_REQUIRED; }
const QString CEncode::name() { return NAME; }

//Public:
Qx::Error CEncode::process(const QStringList& commandLine)
{
    //-Preparation---------------------------------------
    mCore.printMessage(NAME, MSG_COMMAND_INVOCATION);

    // Parse and check for valid arguments
    CommandError parseError = parse(commandLine);
    if(parseError.isValid())
        return parseError;

    // Handle standard options
    if(checkStandardOptions())
        return CEncodeError();

    // Check for required options
    CommandError reqCheck = checkRequiredOptions();
    if(reqCheck.isValid())
    {
        mCore.printError(NAME, reqCheck);
        return reqCheck;
    }

    // Evaluate encoding type
    PxCrypt::Encoder::Encoding aEncoding;
    QString typeStr = mParser.value(CL_OPTION_TYPE);

    auto potentialType = magic_enum::enum_cast<PxCrypt::Encoder::Encoding>(typeStr.toStdString());
    if(potentialType.has_value())
        aEncoding = potentialType.value();
    else
    {
        CEncodeError err = ERR_INVALID_ENCODING.wSpecific(typeStr);
        mCore.printError(NAME, err);
        return err;
    }
    mCore.printMessage(NAME, MSG_ENCODING.arg(ENUM_NAME(aEncoding)));

    // Evaluate BPC
    quint8 aBpc;
    QString bpcStr = mParser.value(CL_OPTION_DENSITY);
    bool autoDensity = bpcStr.compare("auto", Qt::CaseInsensitive) == 0;
    if(autoDensity)
        aBpc = 0;
    else
    {
        bool valid;
        aBpc = bpcStr.toInt(&valid);

        if(!valid)
        {
            CEncodeError err = ERR_INVALID_DENSITY.wSpecific(bpcStr);
            mCore.printError(NAME, err);
            return err;
        }
    }
    mCore.printMessage(NAME, MSG_BPC.arg(aBpc));

    // Get key
    QByteArray aKey = mParser.value(CL_OPTION_KEY).toUtf8();

    // Get input data info
    QFile inputFile(mParser.value(CL_OPTION_INPUT));
    QFileInfo inputFileInfo(inputFile);
    QString aTag = inputFileInfo.fileName();
    QFileInfo mediumInfo(mParser.value(CL_OPTION_MEDIUM));

    // Ensure medium exists
    if(!mediumInfo.exists())
    {
        CEncodeError err = ERR_MEDIUM_DOES_NOT_EXIST;
        mCore.printError(NAME, err);
        return err;
    }

    // Load payload
    QByteArray aPayload;
    Qx::IoOpReport lr = Qx::readBytesFromFile(aPayload, inputFile);
    if(lr.isFailure())
    {
        mCore.printError(NAME, lr);
        return lr;
    }
    mCore.printMessage(NAME, MSG_PAYLOAD_SIZE.arg(Utility::dataStr(aPayload.size())));

    // Setup Job
    Job job{
        .inputInfo = inputFileInfo,
        .mediumInfo = mediumInfo,
        .bpc = aBpc,
        .encoding = aEncoding,
        .payload = aPayload,
        .psk = aKey,
        .tag = aTag
    };

    // Do job
    auto jobError = mediumInfo.isDir() ? encodeMultipleImages(job) : encodeSingleImage(job);
    if(jobError)
    {
        mCore.printError(NAME, jobError);
        return jobError;
    }

    return CEncodeError();
}

