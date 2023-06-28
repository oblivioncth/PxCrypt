#ifndef CDECODE_H
#define CDECODE_H

// Magic enum
#include <magic_enum.hpp>

// Project Includes
#include "command.h"

class QX_ERROR_TYPE(CDecodeError, "CDecodeError", 3001)
{
    friend class CDecode;

//-Class Enums--------------------------------------------------------------------------------------------------------
public:
    enum Type
    {
        NoError = 0,
        FailedReadingMedium = 1,
        FailedReadingInput = 2
    };

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Type mType;
    QString mGeneral;
    QString mSpecific;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    CDecodeError();

private:
    CDecodeError(Type type, const QString& gen);

//-Instance Functions---------------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;

    CDecodeError wSpecific(const QString& spec) const;

public:
    bool isValid() const;
    Type type() const;
    QString errorString() const;
};

class CDecode : public Command
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Error
    static inline const CDecodeError ERR_MEDIUM_READ_FAILED =
            CDecodeError(CDecodeError::FailedReadingMedium, "Failed reading the medium image.");
    static inline const CDecodeError ERR_INPUT_READ_FAILED =
            CDecodeError(CDecodeError::FailedReadingInput, "Failed reading the input encoded image.");

    // Messages
    static inline const QString MSG_COMMAND_INVOCATION = QSL(PROJECT_SHORT_NAME " Decode\n--------------");
    static inline const QString MSG_ENCODING_TYPE = QSL("Encoding: %1");
    static inline const QString MSG_DECODING = QSL("Decoding...");
    static inline const QString MSG_PAYLOAD_SIZE = QSL("Decoded %1 KiB");
    static inline const QString MSG_TAG = QSL("Data tag: %1");
    static inline const QString MSG_DATA_SAVED = QSL("Wrote decoded data to '%1'");

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

    // Command line options
    static inline const QCommandLineOption CL_OPTION_INPUT{{CL_OPT_INPUT_S_NAME, CL_OPT_INPUT_L_NAME}, CL_OPT_INPUT_DESC, "input"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_OUTPUT{{CL_OPT_OUTPUT_S_NAME, CL_OPT_OUTPUT_L_NAME}, CL_OPT_OUTPUT_DESC, "output"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_MEDIUM{{CL_OPT_MEDIUM_S_NAME, CL_OPT_MEDIUM_L_NAME}, CL_OPT_MEDIUM_DESC, "medium"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_KEY{{CL_OPT_KEY_S_NAME, CL_OPT_KEY_L_NAME}, CL_OPT_KEY_DESC, "key", CL_OPT_KEY_DEFAULT}; // Takes value

    static inline const QList<const QCommandLineOption*> CL_OPTIONS_SPECIFIC{&CL_OPTION_INPUT, &CL_OPTION_OUTPUT, &CL_OPTION_MEDIUM,
                                                                             &CL_OPTION_KEY};
    static inline const QSet<const QCommandLineOption*> CL_OPTIONS_REQUIRED{&CL_OPTION_INPUT};

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
    Qx::Error process(const QStringList& commandLine) override;
};
REGISTER_COMMAND(CDecode::NAME, CDecode, CDecode::DESCRIPTION);

#endif // CDECODE_H
