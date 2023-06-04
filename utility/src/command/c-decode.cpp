// Unit Includes
#include "c-decode.h"

// Qt Includes
#include <QImageReader>
#include <QImageWriter>

// Qx Includes
#include <qx/io/qx-common-io.h>

// Project Includes
#include "pxcrypt/decode.h"

//===============================================================================================================
// CDecode
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Public:
CDecode::CDecode(Core& coreRef) : Command(coreRef)
{}

//-Instance Functions-------------------------------------------------------------
//Protected:
const QList<const QCommandLineOption*> CDecode::options() { return CL_OPTIONS_SPECIFIC + Command::options(); }
const QSet<const QCommandLineOption*> CDecode::requiredOptions() { return CL_OPTIONS_REQUIRED; }
const QString CDecode::name() { return NAME; }

//Public:
ErrorCode CDecode::process(const QStringList& commandLine)
{
    //-Preparation---------------------------------------

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

    // Evaluate type
    PxCrypt::EncType aType;
    QString typeStr = mParser.value(CL_OPTION_TYPE);

    auto potentialType = magic_enum::enum_cast<PxCrypt::EncType>(typeStr.toStdString());
    if(potentialType.has_value())
        aType = potentialType.value();
    else
    {
        mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, Core::ERR_INVALID_PARAM, ERR_INVALID_ENC_TYPE));
        return ErrorCode::INVALID_ARGS;
    }
    mCore.printMessage(NAME, MSG_ENCODING_TYPE.arg(ENUM_NAME(aType)));

    // Ensure medium was provided if type is Relative
    if(aType == PxCrypt::EncType::Relative)
    {
        if(!mParser.isSet(CL_OPTION_MEDIUM))
        {
            mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, Core::ERR_INVALID_PARAM, ERR_MISSING_MEDIUM));
            return ErrorCode::INVALID_ARGS;
        }
    }

    // Get key
    QString aKey = mParser.value(CL_OPTION_KEY);

    // Get input file info
    QString encodedPath(mParser.value(CL_OPTION_INPUT));
    QFileInfo encodedFileInfo(encodedPath);

    //-Decoding---------------------------------------
    QImageReader imgReader;

    // Load medium if applicable
    QImage aMedium;
    if(aType == PxCrypt::EncType::Relative)
    {
        QString mediumPath = mParser.value(CL_OPTION_MEDIUM);
        imgReader.setFileName(mediumPath);
        if(!imgReader.read(&aMedium))
        {
            mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, ERR_MEDIUM_READ_FAILED, imgReader.errorString()));
            return ErrorCode::ENCODE_FAILED;
        }
    }

    // Load encoded image
    QImage aEncoded;
    imgReader.setFileName(encodedPath);
    if(!imgReader.read(&aEncoded))
    {
        mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, ERR_MEDIUM_READ_FAILED, imgReader.errorString()));
        return ErrorCode::ENCODE_FAILED;
    }

    // Decode
    PxCrypt::DecodeSettings ds{
        .psk = aKey,
        .type = aType
    };
    QString tag;
    QByteArray decoded;

    mCore.printMessage(NAME, MSG_DECODING);
    Qx::GenericError decError = PxCrypt::decode(decoded, tag, aEncoded, ds, aMedium);
    if(decError.isValid())
    {
        mCore.printError(NAME, decError);
        return ErrorCode::ENCODE_FAILED;
    }

    mCore.printMessage(NAME, MSG_PAYLOAD_SIZE.arg(decoded.size()/1024.0, 0, 'g', 2));
    mCore.printMessage(NAME, MSG_TAG.arg(tag));

    // Write decoded data
    QDir outputDir(mParser.isSet(CL_OPTION_OUTPUT) ? mParser.value(CL_OPTION_OUTPUT) : encodedFileInfo.absoluteDir());
    QFile outputFile(outputDir.absoluteFilePath(tag));

    Qx::IoOpReport wr = Qx::writeBytesToFile(outputFile, decoded, Qx::WriteMode::Truncate, 0, Qx::WriteOption::NewOnly);
    if(wr.isFailure())
    {
        mCore.printError(NAME, wr.toGenericError().setErrorLevel(Qx::GenericError::Error));
        return ErrorCode::ENCODE_FAILED;
    }
    mCore.printMessage(NAME, MSG_DATA_SAVED.arg(outputFile.fileName()));

    return ErrorCode::NO_ERR;
}
