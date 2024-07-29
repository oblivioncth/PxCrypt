// Unit Includes
#include "c-measure.h"

// Qt Includes
#include <QFileInfo>
#include <QImageReader>

// Project Includes
#include "pxcrypt/codec/standard_encoder.h"

//===============================================================================================================
// CMeasureError
//===============================================================================================================

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
CMeasureError::CMeasureError() :
    mType(NoError)
{}

//Private:
CMeasureError::CMeasureError(Type type, const QString& gen) :
    mType(type),
    mGeneral(gen)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint32 CMeasureError::deriveValue() const { return static_cast<quint32>(mType); }
QString CMeasureError::derivePrimary() const { return mGeneral; }
QString CMeasureError::deriveSecondary() const { return mSpecific; }
CMeasureError CMeasureError::wSpecific(const QString& spec) const
{
    CMeasureError s = *this;
    s.mSpecific = spec;
    return s;
}

//Public:
bool CMeasureError::isValid() const { return mType != NoError; }
CMeasureError::Type CMeasureError::type() const { return mType; }
QString CMeasureError::errorString() const { return mGeneral + " " + mSpecific; }

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
Qx::Error CMeasure::process(const QStringList& commandLine)
{
    //-Preparation---------------------------------------
    mCore.printMessage(NAME, MSG_COMMAND_INVOCATION);

    // Parse and check for valid arguments
    CommandError parseError = parse(commandLine);
    if(parseError.isValid())
        return parseError;

    // Handle standard options
    if(checkStandardOptions())
        return CMeasureError();

    // Check for required options
    CommandError reqCheck = checkRequiredOptions();
    if(reqCheck.isValid())
    {
        mCore.printError(NAME, reqCheck);
        return reqCheck;
    }

    // Get input file info
    QFileInfo imageInfo(mParser.value(CL_OPTION_INPUT));

    // Get image size
    QImageReader imageReader(imageInfo.absoluteFilePath());
    QSize imageSize = imageReader.size();

    if(!imageSize.isValid())
    {
        CMeasureError err = ERR_INPUT_READ_FAILED.wSpecific(imageReader.errorString());
        mCore.printError(NAME, err);
        return err;
    }

    // Get filename
    QString filename = mParser.value(CL_OPTION_FILENAME);

    // Determine capacity for all BPC values
    QList<quint64> capacities(7);

    for(quint8 bpc = 1; bpc <= 7; bpc++)
        capacities[bpc - 1] = PxCrypt::StandardEncoder::calculateMaximumPayload(imageSize, filename.size(), bpc);

    // Check if image can't fit anything
    if(*std::max_element(capacities.cbegin(), capacities.cend()) == 0)
    {
        CMeasureError err = ERR_INPUT_TOO_SMALL;
        mCore.printError(NAME, err);
        return err;
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

    return CMeasureError();
}
