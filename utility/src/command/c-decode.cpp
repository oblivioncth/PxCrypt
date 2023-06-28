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
CDecodeError CDecodeError::wSpecific(const QString& spec) const
{
    CDecodeError s = *this;
    s.mSpecific = spec;
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
//Protected:
const QList<const QCommandLineOption*> CDecode::options() { return Command::options() + CL_OPTIONS_SPECIFIC; }
const QSet<const QCommandLineOption*> CDecode::requiredOptions() { return CL_OPTIONS_REQUIRED; }
const QString CDecode::name() { return NAME; }

//Public:
Qx::Error CDecode::process(const QStringList& commandLine)
{
    //-Preparation---------------------------------------
    mCore.printMessage(NAME, MSG_COMMAND_INVOCATION);

    // Parse and check for valid arguments
    CommandError parseError = parse(commandLine);
    if(parseError.isValid())
        return parseError;

    // Handle standard options
    if(checkStandardOptions())
        return CDecodeError();

    // Check for required options
    CommandError reqCheck = checkRequiredOptions();
    if(reqCheck.isValid())
    {
        mCore.printError(NAME, reqCheck);
        return reqCheck;
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
            CDecodeError err = ERR_MEDIUM_READ_FAILED.wSpecific(imgReader.errorString());
            mCore.printError(NAME, err);
            return err;
        }
    }

    // Load encoded image
    QImage aEncoded;
    imgReader.setFileName(encodedPath);
    if(!imgReader.read(&aEncoded))
    {
        CDecodeError err = ERR_INPUT_READ_FAILED.wSpecific(imgReader.errorString());
        mCore.printError(NAME, err);
        return err;
    }

    // Decode
    PxCrypt::Decoder decoder;
    decoder.setPresharedKey(aKey);

    mCore.printMessage(NAME, MSG_DECODING);
    QByteArray decoded = decoder.decode(aEncoded, aMedium);
    if(decoder.hasError())
    {
        PxCrypt::DecodeError err = decoder.error();
        mCore.printError(NAME, err);
        return err;
    }

    mCore.printMessage(NAME, MSG_PAYLOAD_SIZE.arg(decoded.size()/1024.0, 0, 'f', 2));
    mCore.printMessage(NAME, MSG_TAG.arg(decoder.tag()));

    // Write decoded data
    QDir outputDir(mParser.isSet(CL_OPTION_OUTPUT) ? mParser.value(CL_OPTION_OUTPUT) : encodedFileInfo.absoluteDir());
    QFile outputFile(outputDir.absoluteFilePath(decoder.tag()));

    Qx::IoOpReport wr = Qx::writeBytesToFile(outputFile, decoded, Qx::WriteMode::Truncate, 0, Qx::WriteOption::NewOnly);
    if(wr.isFailure())
    {
        mCore.printError(NAME, wr);
        return wr;
    }
    mCore.printMessage(NAME, MSG_DATA_SAVED.arg(outputFile.fileName()));

    return CDecodeError();
}
