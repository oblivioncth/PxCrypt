// Unit Includes
#include "c-encode.h"

// Qt Includes
#include <QImageReader>
#include <QImageWriter>

// Qx Includes
#include <qx/io/qx-common-io.h>

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
ErrorCode CEncode::process(const QStringList& commandLine)
{
    //-Preparation---------------------------------------
    mCore.printMessage(NAME, MSG_COMMAND_INVOCATION);

    // Parse and check for valid arguments
    ErrorCode parseError = parse(commandLine);
    if(parseError)
        return parseError;

    // Handle standard options
    if(checkStandardOptions())
        return ErrorCode::NO_ERR;

    // Check for required options
    Qx::GenericError reqCheck = checkRequiredOptions();
    if(reqCheck.isValid())
    {
        mCore.printError(NAME, reqCheck);
        return ErrorCode::INVALID_ARGS;
    }

    // Evaluate encoding type
    PxCrypt::Encoder::Encoding aEncoding;
    QString typeStr = mParser.value(CL_OPTION_TYPE);

    auto potentialType = magic_enum::enum_cast<PxCrypt::Encoder::Encoding>(typeStr.toStdString());
    if(potentialType.has_value())
        aEncoding = potentialType.value();
    else
    {
        mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, Core::ERR_INVALID_PARAM, ERR_INVALID_ENCODING));
        return ErrorCode::INVALID_ARGS;
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
            mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, Core::ERR_INVALID_PARAM, ERR_INVALID_DENSITY));
            return ErrorCode::INVALID_ARGS;
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
        mCore.printError(NAME, lr.toGenericError().setErrorLevel(Qx::GenericError::Error));
        return ErrorCode::ENCODE_FAILED;
    }
    mCore.printMessage(NAME, MSG_PAYLOAD_SIZE.arg(aPayload.size()/1024.0, 0, 'f', 2));

    // Load medium
    QImageReader imgReader(mediumPath);
    QImage aMedium;
    if(!imgReader.read(&aMedium))
    {
        mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, ERR_MEDIUM_READ_FAILED, imgReader.errorString()));
        return ErrorCode::ENCODE_FAILED;
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
        mCore.printError(NAME, encoder.error());
        return ErrorCode::ENCODE_FAILED;
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
        mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, ERR_OUTPUT_WRITE_FAILED, ERR_OUTPUT_ALREADY_EXISTS));
        return ErrorCode::ENCODE_FAILED;
    }

    QImageWriter imgWriter(outputPath);
    if(!imgWriter.write(encoded))
    {
        mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, ERR_OUTPUT_WRITE_FAILED, imgWriter.errorString()));
        return ErrorCode::ENCODE_FAILED;
    }
    mCore.printMessage(NAME, MSG_IMAGE_SAVED.arg(outputPath));

    return ErrorCode::NO_ERR;
}

