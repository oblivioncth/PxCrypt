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
            CDecodeError(CDecodeError::FailedReadingMedium, u"Failed reading the medium image."_s);
    static inline const CDecodeError ERR_INPUT_READ_FAILED =
            CDecodeError(CDecodeError::FailedReadingInput, u"Failed reading the input encoded image."_s);

    // Messages
    static inline const QString MSG_COMMAND_INVOCATION = PROJECT_SHORT_NAME u" Decode\n--------------"_s;
    static inline const QString MSG_ENCODING_TYPE = u"Encoding: %1"_s;
    static inline const QString MSG_DECODING = u"Decoding..."_s;
    static inline const QString MSG_PAYLOAD_SIZE = u"Decoded %1 KiB"_s;
    static inline const QString MSG_TAG = u"Data tag: %1"_s;
    static inline const QString MSG_DATA_SAVED = u"Wrote decoded data to '%1'"_s;

    // Command line option strings
    static inline const QString CL_OPT_INPUT_S_NAME = u"i"_s;
    static inline const QString CL_OPT_INPUT_L_NAME = u"input"_s;
    static inline const QString CL_OPT_INPUT_DESC = u"Path to encoded image for decoding."_s;

    static inline const QString CL_OPT_OUTPUT_S_NAME = u"o"_s;
    static inline const QString CL_OPT_OUTPUT_L_NAME = u"output"_s;
    static inline const QString CL_OPT_OUTPUT_DESC = u"Directory to place the decoded output. Defaults to the input path's directory."_s;

    static inline const QString CL_OPT_MEDIUM_S_NAME = u"m"_s;
    static inline const QString CL_OPT_MEDIUM_L_NAME = u"medium"_s;
    static inline const QString CL_OPT_MEDIUM_DESC = u"Original image used for encoding (required for Relative type)."_s;

    static inline const QString CL_OPT_KEY_S_NAME = u"k"_s;
    static inline const QString CL_OPT_KEY_L_NAME = u"key"_s;
    static inline const QString CL_OPT_KEY_DESC = u"The key for key-protected images (optional)."_s;
    static inline const QString CL_OPT_KEY_DEFAULT = u""_s;

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
    static inline const QString NAME = u"decode"_s;
    static inline const QString DESCRIPTION = u"Retrieve the original data from an encoded image."_s;

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
