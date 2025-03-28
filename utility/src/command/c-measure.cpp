// Unit Includes
#include "c-measure.h"

// Qt Includes
#include <QFileInfo>
#include <QImageReader>

// Qx Includes
#include <qx/io/qx-common-io.h>

// Project Includes
#include "pxcrypt/codec/standard_encoder.h"
#include "pxcrypt/codec/multi_encoder.h"
#include "utility.h"

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
QString CMeasureError::deriveDetails() const { return mDetails; }

CMeasureError CMeasureError::wSpecific(const QString& spec, const QString& det) const
{
    CMeasureError s = *this;
    s.mSpecific = spec;
    s.mDetails = det;
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
//Private:
Qx::Error CMeasure::measureSingleImage(QList<quint64>& capacities, const QString& imgPath, const QString& filename)
{
    // Get image size
    QImageReader imageReader(imgPath);
    QSize imageSize = imageReader.size();

    if(!imageSize.isValid())
        CMeasureError err = ERR_INPUT_READ_FAILED.wSpecific(imageReader.errorString());

    // Determine capacity for all BPC values
    capacities.resize(7);
    for(quint8 bpc = 1; bpc <= 7; bpc++)
        capacities[bpc - 1] = PxCrypt::StandardEncoder::calculateMaximumPayload(imageSize, filename.size(), bpc);

    mCore.printMessage(NAME, MSG_SINGLE_IMAGE_DIMENSIONS.arg(imageSize.width()).arg(imageSize.height()));
    return {};
}

Qx::Error CMeasure::measureMultipleImages(QList<quint64>& capacities, const QDir& imgRoot, const QString& filename)
{
    // Get list of mediums
    QStringList imagePaths;
    if(auto rp = Qx::dirContentList(imagePaths, imgRoot, mCore.imageFormatFilter(), QDir::NoFilter, QDirIterator::Subdirectories); rp.isFailure())
        return rp;
    else if(imagePaths.isEmpty())
        return ERR_INPUT_READ_FAILED.wSpecific(u"No images were found in the provided directory."_s);

    // Read all sizes
    QImageReader imageReader;
    QList<QSize> imageSizes(imagePaths.size());
    for(qsizetype i = 0; i < imageSizes.size(); ++i)
    {
        auto& ip = imagePaths.at(i);
        QSize& m = imageSizes[i];
        imageReader.setFileName(ip);
        m = imageReader.size();
        if(!m.isValid())
            return ERR_INPUT_READ_FAILED.wSpecific(imageReader.errorString(), ip);
    }

    // Determine capacity for all BPC values
    capacities.resize(7);
    for(quint8 bpc = 1; bpc <= 7; bpc++)
        capacities[bpc - 1] = PxCrypt::MultiEncoder::calculateMaximumPayload(imageSizes, filename.size(), bpc);

    mCore.printMessage(NAME, MSG_MULTI_IMAGE_COUNT.arg(imageSizes.count()));
    return {};
}

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

    // Get input info
    QFileInfo inputInfo(mParser.value(CL_OPTION_INPUT));
    QString filename(mParser.value(CL_OPTION_FILENAME));

    // Ensure input exists
    if(!inputInfo.exists())
    {
        CMeasureError err = ERR_INPUT_DOES_NOT_EXIST;
        mCore.printError(NAME, err);
        return err;
    }

    // Measure
    QList<quint64> capacities;
    auto measureError = inputInfo.isDir() ? measureMultipleImages(capacities, inputInfo.absoluteFilePath(), filename) :
                                            measureSingleImage(capacities, inputInfo.absoluteFilePath(), filename);
    if(measureError)
    {
        mCore.printError(NAME, measureError);
        return measureError;
    }

    // Check if image can't fit anything
    if(*std::max_element(capacities.cbegin(), capacities.cend()) == 0)
    {
        CMeasureError err = ERR_INPUT_TOO_SMALL;
        mCore.printError(NAME, err);
        return err;
    }

    // Print stats
    mCore.printMessage(NAME, MSG_TAG_CONSUMPTION.arg(Utility::dataStr(filename.size())));
    mCore.printMessage(NAME, MSG_PAYLOAD_CAPACITY);

    // Print capacity at all BPC
    QString measurement;
    for(quint8 bpc = 1; bpc <= 7; bpc++)
    {
        quint64 c = capacities[bpc - 1];
        measurement += MEASUREMENT_LINE.arg(bpc).arg(Utility::dataStr(c)).arg(c);
    }
    mCore.printMessage(NAME, measurement);

    return CMeasureError();
}
