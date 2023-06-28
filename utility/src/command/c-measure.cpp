// Unit Includes
#include "c-measure.h"

// Qt Includes
#include <QFileInfo>
#include <QImageReader>

// Project Includes
#include "pxcrypt/encoder.h"

//===============================================================================================================
// CMeasure
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Public:
CMeasure::CMeasure(Core& coreRef) : Command(coreRef)
{}

//-Instance Functions-------------------------------------------------------------
//Protected:
const QList<const QCommandLineOption*> CMeasure::options() { return Command::options() + CL_OPTIONS_SPECIFIC; }
const QSet<const QCommandLineOption*> CMeasure::requiredOptions() { return CL_OPTIONS_REQUIRED; }
const QString CMeasure::name() { return NAME; }

//Public:
ErrorCode CMeasure::process(const QStringList& commandLine)
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

    // Get input file info
    QFileInfo imageInfo(mParser.value(CL_OPTION_INPUT));

    // Get image size
    QImageReader imageReader(imageInfo.absoluteFilePath());
    QSize imageSize = imageReader.size();

    if(!imageSize.isValid())
    {
        mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, ERR_INPUT_READ_FAILED, imageReader.errorString()));
        return ErrorCode::MEASURE_FAILED;
    }

    // Get filename
    QString filename = mParser.value(CL_OPTION_FILENAME);

    // Determine capacity for all BPC values
    QList<quint64> capacities(7);

    for(quint8 bpc = 1; bpc <= 7; bpc++)
        capacities[bpc - 1] = PxCrypt::Encoder::calculateMaximumStorage(imageSize, filename.size(), bpc);

    // Check if image can't fit anything
    if(*std::max_element(capacities.cbegin(), capacities.cend()) == 0)
    {
        mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, ERR_INPUT_TOO_SMALL));
        return ErrorCode::MEASURE_FAILED;
    }

    // Print stats
    mCore.printMessage(NAME, MSG_IMAGE_DIMENSIONS.arg(imageSize.width()).arg(imageSize.height()));
    mCore.printMessage(NAME, MSG_TAG_CONSUMPTION.arg(filename.size()));
    mCore.printMessage(NAME, MSG_PAYLOAD_CAPACITY);

    // Print capacity at all BPC
    QString measurement;
    for(quint8 bpc = 1; bpc <= 7; bpc++)
    {
        quint64 c = capacities[bpc - 1];
        measurement += MEASUREMENT_LINE.arg(bpc).arg(c).arg(c/1024.0, 0, 'f', 2);
    }
    mCore.printMessage(NAME, measurement);

    return ErrorCode::NO_ERR;
}
