#ifndef CMEASURE_H
#define CMEASURE_H

// Magic enum
#include <magic_enum.hpp>

// Project Includes
#include "command.h"

class CMeasure : public Command
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Measurement
    static inline const QString MEASUREMENT_LINE = QSL("BPC %1 - %2 bytes (%3 KiB)\n");

    // Messages
    static inline const QString MSG_COMMAND_INVOCATION = QSL(PROJECT_SHORT_NAME " Measure\n--------------");
    static inline const QString MSG_IMAGE_DIMENSIONS = QSL("Image Dimmensions: %1 x %2");
    static inline const QString MSG_TAG_CONSUMPTION = QSL("Filename Consumes: %1 bytes");
    static inline const QString MSG_PAYLOAD_CAPACITY = QSL("Payload Capacities:");

    // Error Messages
    static inline const QString ERR_INPUT_READ_FAILED = QSL("Failed reading the input image.");
    static inline const QString ERR_INPUT_TOO_SMALL = QSL("The provided image .");

    // Command line option strings
    static inline const QString CL_OPT_INPUT_S_NAME = QSL("i");
    static inline const QString CL_OPT_INPUT_L_NAME = QSL("input");
    static inline const QString CL_OPT_INPUT_DESC = QSL("Path to image to measure.");

    static inline const QString CL_OPT_FILENAME_S_NAME = QSL("f");
    static inline const QString CL_OPT_FILENAME_L_NAME = QSL("filename");
    static inline const QString CL_OPT_FILENAME_DESC = QSL("Name of potential payload file. Defaults to 'filename.txt'");
    static inline const QString CL_OPT_FILENAME_DEFAULT = QSL("filename.txt");

    // Command line options
    static inline const QCommandLineOption CL_OPTION_INPUT{{CL_OPT_INPUT_S_NAME, CL_OPT_INPUT_L_NAME}, CL_OPT_INPUT_DESC, "input"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_FILENAME{{CL_OPT_FILENAME_S_NAME, CL_OPT_FILENAME_L_NAME}, CL_OPT_FILENAME_DESC, "filename", CL_OPT_FILENAME_DEFAULT}; // Takes value

    static inline const QList<const QCommandLineOption*> CL_OPTIONS_SPECIFIC{&CL_OPTION_INPUT, &CL_OPTION_FILENAME};
    static inline const QSet<const QCommandLineOption*> CL_OPTIONS_REQUIRED{&CL_OPTION_INPUT};

public:
    // Meta
    static inline const QString NAME = QSL("measure");
    static inline const QString DESCRIPTION = QSL("Determine how much data can be encoded within a given image.");

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    CMeasure(Core& coreRef);

//-Instance Functions------------------------------------------------------------------------------------------------------
protected:
    const QList<const QCommandLineOption*> options() override;
    const QSet<const QCommandLineOption*> requiredOptions() override;
    const QString name() override;

public:
    ErrorCode process(const QStringList& commandLine) override;
};
REGISTER_COMMAND(CMeasure::NAME, CMeasure, CMeasure::DESCRIPTION);

#endif // CMEASURE_H
