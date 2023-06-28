// Unit Includes
#include "c-encode.h"

// Qt Includes
#include <QImageReader>
#include <QImageWriter>

// Qx Includes
#include <qx/io/qx-common-io.h>

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
CEncodeError CEncodeError::wSpecific(const QString& spec) const
{
    CEncodeError s = *this;
    s.mSpecific = spec;
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
        CEncodeError err = ERR_INVALID_OPTION.wSpecific(QSL("Invalid encoding."));
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
            CEncodeError err = ERR_INVALID_OPTION.wSpecific(QSL("Invalid data density."));
            mCore.printError(NAME, err);
            return err;
        }
    }

    // Get key
    QByteArray aKey = mParser.value(CL_OPTION_KEY).toUtf8();

    // Get input file info
    QFile inputFile(mParser.value(CL_OPTION_INPUT));
    QFileInfo inputFileInfo(inputFile);
    QString aTag = inputFileInfo.fileName();
    QString mediumPath(mParser.value(CL_OPTION_MEDIUM));

    //-Encoding---------------------------------------

    // Load payload
    QByteArray aPayload;
    Qx::IoOpReport lr = Qx::readBytesFromFile(aPayload, inputFile);
    if(lr.isFailure())
    {
        mCore.printError(NAME, lr);
        return lr;
    }
    mCore.printMessage(NAME, MSG_PAYLOAD_SIZE.arg(aPayload.size()/1024.0, 0, 'f', 2));

    // Load medium
    QImageReader imgReader(mediumPath);
    QImage aMedium;
    if(!imgReader.read(&aMedium))
    {
        CEncodeError err = ERR_MEDIUM_READ_FAILED.wSpecific(imgReader.errorString());
        mCore.printError(NAME, err);
        return err;
    }

    // Print medium size
    QSize mediumSize = aMedium.size();
    mCore.printMessage(NAME, MSG_MEDIUM_DIM.arg(mediumSize.width()).arg(mediumSize.height()));

    // Encode
    PxCrypt::Encoder encoder;
    encoder.setBpc(aBpc);
    encoder.setPresharedKey(aKey);
    encoder.setEncoding(aEncoding);
    encoder.setTag(aTag);

    mCore.printMessage(NAME, MSG_ENCODING);
    QImage encoded = encoder.encode(aPayload, aMedium);
    if(encoder.hasError())
    {
        PxCrypt::EncodeError err = encoder.error();
        mCore.printError(NAME, err);
        return err;
    }

    // Print density (largely for when set to auto)
    mCore.printMessage(NAME, MSG_BPC.arg(encoder.bpc()));

    // Write encoded image
    QString outputPath;
    if(mParser.isSet(CL_OPTION_OUTPUT))
        outputPath = mParser.value(CL_OPTION_OUTPUT);
    else
    {
        QDir dir = inputFileInfo.absoluteDir();
        QString basename = inputFileInfo.baseName();
        outputPath = dir.absoluteFilePath(basename + "_enc." + OUTPUT_EXT);
    }

    if(QFile::exists(outputPath))
    {
        CEncodeError err = ERR_OUTPUT_WRITE_FAILED.wSpecific(QSL("The file already exists."));
        mCore.printError(NAME, err);
        return err;
    }

    QImageWriter imgWriter(outputPath);
    if(!imgWriter.write(encoded))
    {
        CEncodeError err = ERR_OUTPUT_WRITE_FAILED.wSpecific(imgWriter.errorString());
        mCore.printError(NAME, err);
        return err;
    }
    mCore.printMessage(NAME, MSG_IMAGE_SAVED.arg(outputPath));

    return CEncodeError();
}

