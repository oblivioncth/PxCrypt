#ifndef CMEASURE_H
#define CMEASURE_H

// Qt Includes
#include <QDir>

// Magic enum
#include <magic_enum.hpp>

// Project Includes
#include "command.h"

class QX_ERROR_TYPE(CMeasureError, "CMeasureError", 3003)
{
    friend class CMeasure;

//-Class Enums--------------------------------------------------------------------------------------------------------
public:
    enum Type
    {
        NoError,
        InputDoesNotExist,
        FailedReadingInput,
        InputTooSmall
    };

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Type mType;
    QString mGeneral;
    QString mSpecific;
    QString mDetails;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    CMeasureError();

private:
    CMeasureError(Type type, const QString& gen);

//-Instance Functions---------------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    QString deriveDetails() const override;

    CMeasureError wSpecific(const QString& spec, const QString& det = {}) const;

public:
    bool isValid() const;
    Type type() const;
    QString errorString() const;
};

class CMeasure : public Command
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Error
    static inline const CMeasureError ERR_INPUT_DOES_NOT_EXIST =
        CMeasureError(CMeasureError::InputDoesNotExist, u"The provided input path does not exist."_s);
    static inline const CMeasureError ERR_INPUT_READ_FAILED =
        CMeasureError(CMeasureError::FailedReadingInput, u"Failed reading the input image."_s);
    static inline const CMeasureError ERR_INPUT_TOO_SMALL =
        CMeasureError(CMeasureError::InputTooSmall, u"The provided image's dimensions are too small for it to act as a medium."_s);


    // Measurement
    static inline const QString MEASUREMENT_LINE = u"BPC %1 - %2 bytes (%3 KiB)\n"_s;

    // Messages - All
    static inline const QString MSG_COMMAND_INVOCATION = PROJECT_SHORT_NAME u" Measure\n--------------"_s;
    static inline const QString MSG_TAG_CONSUMPTION = u"Tag Consumes: %1 bytes"_s;
    static inline const QString MSG_PAYLOAD_CAPACITY = u"Payload Capacities:"_s;

    // Messages - Single
    static inline const QString MSG_SINGLE_IMAGE_DIMENSIONS = u"Image Dimensions: %1 x %2"_s;

    // Messages - Multi
    static inline const QString MSG_MULTI_IMAGE_COUNT = u"Image Count: %1"_s;

    // Command line option strings
    static inline const QString CL_OPT_INPUT_S_NAME = u"i"_s;
    static inline const QString CL_OPT_INPUT_L_NAME = u"input"_s;
    static inline const QString CL_OPT_INPUT_DESC = u"Path to image(s) to measure. A path to a directory evaluates all images within for a multi-part encode."_s;

    static inline const QString CL_OPT_FILENAME_S_NAME = u"f"_s;
    static inline const QString CL_OPT_FILENAME_L_NAME = u"filename"_s;
    static inline const QString CL_OPT_FILENAME_DESC = u"Name of potential payload file. Defaults to 'filename.txt'"_s;
    static inline const QString CL_OPT_FILENAME_DEFAULT = u"filename.txt"_s;

    // Command line options
    static inline const QCommandLineOption CL_OPTION_INPUT{{CL_OPT_INPUT_S_NAME, CL_OPT_INPUT_L_NAME}, CL_OPT_INPUT_DESC, "input"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_FILENAME{{CL_OPT_FILENAME_S_NAME, CL_OPT_FILENAME_L_NAME}, CL_OPT_FILENAME_DESC, "filename", CL_OPT_FILENAME_DEFAULT}; // Takes value

    static inline const QList<const QCommandLineOption*> CL_OPTIONS_SPECIFIC{&CL_OPTION_INPUT, &CL_OPTION_FILENAME};
    static inline const QSet<const QCommandLineOption*> CL_OPTIONS_REQUIRED{&CL_OPTION_INPUT};

public:
    // Meta
    static inline const QString NAME = u"measure"_s;
    static inline const QString DESCRIPTION = u"Determine how much data can be encoded within a given image."_s;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    CMeasure(Core& coreRef);

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    Qx::Error measureSingleImage(QList<quint64>& capacities, const QString& imgPath, const QString& filename);
    Qx::Error measureMultipleImages(QList<quint64>& capacities, const QDir& imgRoot, const QString& filename);

protected:
    const QList<const QCommandLineOption*> options() override;
    const QSet<const QCommandLineOption*> requiredOptions() override;
    const QString name() override;

public:
    Qx::Error process(const QStringList& commandLine) override;
};
REGISTER_COMMAND(CMeasure::NAME, CMeasure, CMeasure::DESCRIPTION);

#endif // CMEASURE_H
