#ifndef CDECODE_H
#define CDECODE_H

// Qt Includes

// Magic enum
#include <magic_enum.hpp>

// Project Includes
#include "command.h"
#include "pxcrypt/encdec.h"

class CDecode : public Command
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Messages
    static inline const QString MSG_ENCODING_TYPE = QSL("Encoding Type: %1");
    static inline const QString MSG_DECODING = QSL("Decoding...");
    static inline const QString MSG_PAYLOAD_SIZE = QSL("Decoded %1 KiB");
    static inline const QString MSG_TAG = QSL("Data tag: %1");
    static inline const QString MSG_DATA_SAVED = QSL("Wrote decoded data to '%1'");

    // Error Messages
    static inline const QString ERR_INVALID_ENC_TYPE = QSL("Invalid encoding type.");
    static inline const QString ERR_MISSING_MEDIUM = QSL("A medium is required to decode Relative type images.");
    static inline const QString ERR_MEDIUM_READ_FAILED = QSL("Failed reading the medium image.");
    static inline const QString ERR_INPUT_READ_FAILED = QSL("Failed reading the input encoded image.");

    // Command line option strings
    static inline const QString CL_OPT_INPUT_S_NAME = QSL("i");
    static inline const QString CL_OPT_INPUT_L_NAME = QSL("input");
    static inline const QString CL_OPT_INPUT_DESC = QSL("Path to encoded image for decoding.");

    static inline const QString CL_OPT_OUTPUT_S_NAME = QSL("o");
    static inline const QString CL_OPT_OUTPUT_L_NAME = QSL("output");
    static inline const QString CL_OPT_OUTPUT_DESC = QSL("Directory to place the decoded output. Defaults to the input path's directory.");

    static inline const QString CL_OPT_MEDIUM_S_NAME = QSL("m");
    static inline const QString CL_OPT_MEDIUM_L_NAME = QSL("medium");
    static inline const QString CL_OPT_MEDIUM_DESC = QSL("Original image used for encoding (required for Relative type).");

    static inline const QString CL_OPT_KEY_S_NAME = QSL("k");
    static inline const QString CL_OPT_KEY_L_NAME = QSL("key");
    static inline const QString CL_OPT_KEY_DESC = QSL("The key for key-protected images (optional).");
    static inline const QString CL_OPT_KEY_DEFAULT = QSL("");

    static inline const QString CL_OPT_TYPE_S_NAME = QSL("t");
    static inline const QString CL_OPT_TYPE_L_NAME = QSL("type");
    static inline const QString CL_OPT_TYPE_DESC = QSL(
        "The type of encoding used by the input (defaults to Relative):\n"
        "\n"
        "Relative - Doesn't require the original medium\n"
        "Absolute - Requires the original medium"
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
    static inline const QCommandLineOption CL_OPTION_KEY{{CL_OPT_KEY_S_NAME, CL_OPT_KEY_L_NAME}, CL_OPT_KEY_DESC, "key", CL_OPT_KEY_DEFAULT}; // Takes value
    static inline const QCommandLineOption CL_OPTION_TYPE{{CL_OPT_TYPE_S_NAME, CL_OPT_TYPE_L_NAME}, CL_OPT_TYPE_DESC, "type", CL_OPT_TYPE_DEFAULT}; // Takes value

    static inline const QList<const QCommandLineOption*> CL_OPTIONS_SPECIFIC{&CL_OPTION_INPUT, &CL_OPTION_OUTPUT, &CL_OPTION_MEDIUM,
                                                                             &CL_OPTION_KEY, &CL_OPTION_TYPE};
    static inline const QSet<const QCommandLineOption*> CL_OPTIONS_REQUIRED{&CL_OPTION_INPUT, &CL_OPTION_MEDIUM};

public:
    // Meta
    static inline const QString NAME = QSL("decode");
    static inline const QString DESCRIPTION = QSL("Retrieve the original data from an encoded image.");

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    CDecode(Core& coreRef);

//-Instance Functions------------------------------------------------------------------------------------------------------
protected:
    const QList<const QCommandLineOption*> options() override;
    const QSet<const QCommandLineOption*> requiredOptions() override;
    const QString name() override;

public:
    ErrorCode process(const QStringList& commandLine) override;
};
REGISTER_COMMAND(CDecode::NAME, CDecode, CDecode::DESCRIPTION);

#endif // CDECODE_H
