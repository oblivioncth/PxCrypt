#ifndef CENCODE_H
#define CENCODE_H

// Magic enum
#include <magic_enum.hpp>

// Project Includes
#include "command.h"
#include "pxcrypt/encdec.h"
#include "project_vars.h"

class CEncode : public Command
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Encoding
    static inline const QString OUTPUT_EXT = QSL("png");

    // Messages
    static inline const QString MSG_COMMAND_INVOCATION = QSL(PROJECT_SHORT_NAME " Encode\n--------------");
    static inline const QString MSG_ENCODING_TYPE = QSL("Encoding Type: %1");
    static inline const QString MSG_BPC = QSL("Bits per channel: %1");
    static inline const QString MSG_PAYLOAD_SIZE = QSL("Payload size: %1 KiB");
    static inline const QString MSG_MEDIUM_DIM = QSL("Medium dimmensions: %1 x %2");
    static inline const QString MSG_ENCODING = QSL("Encoding...");
    static inline const QString MSG_IMAGE_SAVED = QSL("Wrote encoded image to '%1'");

    // Error Messages
    static inline const QString ERR_INVALID_ENC_TYPE = QSL("Invalid encoding type.");
    static inline const QString ERR_INVALID_DENSITY = QSL("Invalid data density.");
    static inline const QString ERR_MEDIUM_READ_FAILED = QSL("Failed reading the medium image.");
    static inline const QString ERR_OUTPUT_WRITE_FAILED = QSL("Failed writing the encoded image.");
    static inline const QString ERR_OUTPUT_ALREADY_EXISTS = QSL("The file already exists.");

    // Command line option strings
    static inline const QString CL_OPT_INPUT_S_NAME = QSL("i");
    static inline const QString CL_OPT_INPUT_L_NAME = QSL("input");
    static inline const QString CL_OPT_INPUT_DESC = QSL("Path to input file for encoding.");

    static inline const QString CL_OPT_OUTPUT_S_NAME = QSL("o");
    static inline const QString CL_OPT_OUTPUT_L_NAME = QSL("output");
    static inline const QString CL_OPT_OUTPUT_DESC = QSL("Path to encoded output file. Defaults to the input path with a '_enc' suffix.");

    static inline const QString CL_OPT_MEDIUM_S_NAME = QSL("m");
    static inline const QString CL_OPT_MEDIUM_L_NAME = QSL("medium");
    static inline const QString CL_OPT_MEDIUM_DESC = QSL("The image to encode the data within.");

    static inline const QString CL_OPT_DENSITY_S_NAME = QSL("d");
    static inline const QString CL_OPT_DENSITY_L_NAME = QSL("density");
    static inline const QString CL_OPT_DENSITY_DESC = QSL("How many bits-per-channel to use when encoding the image (auto | 1-7). "
                                                          "Defaults to 'auto'");
    static inline const QString CL_OPT_DENSITY_DEFAULT = QSL("auto");

    static inline const QString CL_OPT_KEY_S_NAME = QSL("k");
    static inline const QString CL_OPT_KEY_L_NAME = QSL("key");
    static inline const QString CL_OPT_KEY_DESC = QSL("An optional key to require in order to decode the encoded image.");
    static inline const QString CL_OPT_KEY_DEFAULT = QSL("");

    static inline const QString CL_OPT_TYPE_S_NAME = QSL("t");
    static inline const QString CL_OPT_TYPE_L_NAME = QSL("type");
    static inline const QString CL_OPT_TYPE_DESC = QSL(
        "The type of encoding to use (defaults to Relative):\n"
        "\n"
        "Relative - Requires the original medium to decode, slightly smaller impact on the image\n"
        "Absolute - Doesn't require the original medium to decode\n"
    );
    static inline const QString CL_OPT_TYPE_DEFAULT = QSL("Relative");

    /* NOTE: This will cause a compilation error when changing PxCrypt::EncType in order to prompt the developer
     * to ensure any new options have been described above and then manually check them off here
     */
    static_assert(magic_enum::enum_values<PxCrypt::EncType>() == std::array<PxCrypt::EncType, 2>{
            PxCrypt::EncType::Relative,
            PxCrypt::EncType::Absolute
        },
        "Missing description for a encoding type"
    );

    // Command line options
    static inline const QCommandLineOption CL_OPTION_INPUT{{CL_OPT_INPUT_S_NAME, CL_OPT_INPUT_L_NAME}, CL_OPT_INPUT_DESC, "input"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_OUTPUT{{CL_OPT_OUTPUT_S_NAME, CL_OPT_OUTPUT_L_NAME}, CL_OPT_OUTPUT_DESC, "output"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_MEDIUM{{CL_OPT_MEDIUM_S_NAME, CL_OPT_MEDIUM_L_NAME}, CL_OPT_MEDIUM_DESC, "medium"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_DENSITY{{CL_OPT_DENSITY_S_NAME, CL_OPT_DENSITY_L_NAME}, CL_OPT_DENSITY_DESC, "density", CL_OPT_DENSITY_DEFAULT}; // Takes value
    static inline const QCommandLineOption CL_OPTION_KEY{{CL_OPT_KEY_S_NAME, CL_OPT_KEY_L_NAME}, CL_OPT_KEY_DESC, "key", CL_OPT_KEY_DEFAULT}; // Takes value
    static inline const QCommandLineOption CL_OPTION_TYPE{{CL_OPT_TYPE_S_NAME, CL_OPT_TYPE_L_NAME}, CL_OPT_TYPE_DESC, "type", CL_OPT_TYPE_DEFAULT}; // Takes value

    static inline const QList<const QCommandLineOption*> CL_OPTIONS_SPECIFIC{&CL_OPTION_INPUT, &CL_OPTION_OUTPUT, &CL_OPTION_MEDIUM,
                                                                             &CL_OPTION_DENSITY, &CL_OPTION_KEY, &CL_OPTION_TYPE};
    static inline const QSet<const QCommandLineOption*> CL_OPTIONS_REQUIRED{&CL_OPTION_INPUT, &CL_OPTION_MEDIUM};

public:
    // Meta
    static inline const QString NAME = QSL("encode");
    static inline const QString DESCRIPTION = QSL("Store a file within the color channel data of an image.");

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    CEncode(Core& coreRef);

//-Instance Functions------------------------------------------------------------------------------------------------------
protected:
    const QList<const QCommandLineOption*> options() override;
    const QSet<const QCommandLineOption*> requiredOptions() override;
    const QString name() override;

public:
    ErrorCode process(const QStringList& commandLine) override;
};
REGISTER_COMMAND(CEncode::NAME, CEncode, CEncode::DESCRIPTION);

#endif // CENCODE_H
