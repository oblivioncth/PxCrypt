// Unit Includes
#include "c-decode.h"

// Qt Includes
#include <QImageReader>
#include <QImageWriter>

// Qx Includes
#include <qx/io/qx-common-io.h>

// Project Includes
#include "pxcrypt/decoder.h"

//===============================================================================================================
// CDecode
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Public:
CDecode::CDecode(Core& coreRef) : Command(coreRef)
{}

//-Instance Functions-------------------------------------------------------------
//Protected:
const QList<const QCommandLineOption*> CDecode::options() { return Command::options() + CL_OPTIONS_SPECIFIC; }
const QSet<const QCommandLineOption*> CDecode::requiredOptions() { return CL_OPTIONS_REQUIRED; }
const QString CDecode::name() { return NAME; }

//Public:
ErrorCode CDecode::process(const QStringList& commandLine)
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

    // Get key
    QByteArray aKey = mParser.value(CL_OPTION_KEY).toUtf8();

    // Get input file info
    QString encodedPath(mParser.value(CL_OPTION_INPUT));
    QFileInfo encodedFileInfo(encodedPath);

    //-Decoding---------------------------------------
    QImageReader imgReader;

    // Load medium if provided
    QImage aMedium;
    if(mParser.isSet(CL_OPTION_MEDIUM))
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
    PxCrypt::Decoder decoder;
    decoder.setPresharedKey(aKey);

    mCore.printMessage(NAME, MSG_DECODING);
    QByteArray decoded = decoder.decode(aEncoded, aMedium);
    if(decoder.hasError())
    {
        mCore.printError(NAME, decoder.error());
        return ErrorCode::ENCODE_FAILED;
    }

    mCore.printMessage(NAME, MSG_PAYLOAD_SIZE.arg(decoded.size()/1024.0, 0, 'f', 2));
    mCore.printMessage(NAME, MSG_TAG.arg(decoder.tag()));

    // Write decoded data
    QDir outputDir(mParser.isSet(CL_OPTION_OUTPUT) ? mParser.value(CL_OPTION_OUTPUT) : encodedFileInfo.absoluteDir());
    QFile outputFile(outputDir.absoluteFilePath(decoder.tag()));

    Qx::IoOpReport wr = Qx::writeBytesToFile(outputFile, decoded, Qx::WriteMode::Truncate, 0, Qx::WriteOption::NewOnly);
    if(wr.isFailure())
    {
        mCore.printError(NAME, wr.toGenericError().setErrorLevel(Qx::GenericError::Error));
        return ErrorCode::ENCODE_FAILED;
    }
    mCore.printMessage(NAME, MSG_DATA_SAVED.arg(outputFile.fileName()));

    return ErrorCode::NO_ERR;
}
