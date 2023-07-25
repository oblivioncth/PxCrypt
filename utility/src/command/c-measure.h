#ifndef CMEASURE_H
#define CMEASURE_H

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
        NoError = 0,
        FailedReadingInput = 1,
        InputTooSmall = 2
    };

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Type mType;
    QString mGeneral;
    QString mSpecific;

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

    CMeasureError wSpecific(const QString& spec) const;

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
    static inline const CMeasureError ERR_INPUT_READ_FAILED =
            CMeasureError(CMeasureError::FailedReadingInput, u"Failed reading the input image."_s);
    static inline const CMeasureError ERR_INPUT_TOO_SMALL =
            CMeasureError(CMeasureError::InputTooSmall, u"The provided image's dimensions are too small for it to act as a medium."_s);

    // Measurement
    static inline const QString MEASUREMENT_LINE = u"BPC %1 - %2 bytes (%3 KiB)\n"_s;

    // Messages
    static inline const QString MSG_COMMAND_INVOCATION = PROJECT_SHORT_NAME u" Measure\n--------------"_s;
    static inline const QString MSG_IMAGE_DIMENSIONS = u"Image Dimmensions: %1 x %2"_s;
    static inline const QString MSG_TAG_CONSUMPTION = u"Filename Consumes: %1 bytes"_s;
    static inline const QString MSG_PAYLOAD_CAPACITY = u"Payload Capacities:"_s;

    // Command line option strings
    static inline const QString CL_OPT_INPUT_S_NAME = u"i"_s;
    static inline const QString CL_OPT_INPUT_L_NAME = u"input"_s;
    static inline const QString CL_OPT_INPUT_DESC = u"Path to image to measure."_s;

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
protected:
    const QList<const QCommandLineOption*> options() override;
    const QSet<const QCommandLineOption*> requiredOptions() override;
    const QString name() override;

public:
    Qx::Error process(const QStringList& commandLine) override;
};
REGISTER_COMMAND(CMeasure::NAME, CMeasure, CMeasure::DESCRIPTION);

#endif // CMEASURE_H
