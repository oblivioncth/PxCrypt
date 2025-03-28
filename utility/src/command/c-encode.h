#ifndef CENCODE_H
#define CENCODE_H

// Magic enum
#include <magic_enum.hpp>

// Qt Includes
#include <QFileInfo>

// Project Includes
#include "command.h"
#include "project_vars.h"
#include "pxcrypt/codec/encoder.h"

class QX_ERROR_TYPE(CEncodeError, "CEncodeError", 3002)
{
    friend class CEncode;

//-Class Enums--------------------------------------------------------------------------------------------------------
public:
    enum Type
    {
        NoError,
        InvalidEncoding,
        InvalidDensity,
        MediumDoesNotExist,
        FailedReadingMedium,
        FailedWritingEncoded
    };

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Type mType;
    QString mGeneral;
    QString mSpecific;
    QString mDetails;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    CEncodeError();

private:
    CEncodeError(Type type, const QString& gen);

//-Instance Functions---------------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    QString deriveDetails() const override;

    CEncodeError wSpecific(const QString& spec, const QString& det = {}) const;

public:
    bool isValid() const;
    Type type() const;
    QString errorString() const;
};

class CEncode : public Command
{
//-Structs--------------------------------------------------------------------------------------------------------------
private:
    struct Job
    {
        QFileInfo inputInfo;
        QFileInfo mediumInfo;
        quint8 bpc;
        PxCrypt::Encoder::Encoding encoding;
        QByteArrayView payload;
        QByteArrayView psk;
        QString tag;
    };

//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Error
    static inline const CEncodeError ERR_INVALID_ENCODING =
        CEncodeError(CEncodeError::InvalidEncoding, u"Invalid encoding:"_s);
    static inline const CEncodeError ERR_INVALID_DENSITY =
        CEncodeError(CEncodeError::InvalidDensity, u"Invalid data density:"_s);
    static inline const CEncodeError ERR_MEDIUM_DOES_NOT_EXIST =
        CEncodeError(CEncodeError::MediumDoesNotExist, u"The provided medium path does not exist."_s);
    static inline const CEncodeError ERR_MEDIUM_READ_FAILED =
        CEncodeError(CEncodeError::FailedReadingMedium, u"Failed reading the medium image(s)."_s);
    static inline const CEncodeError ERR_OUTPUT_WRITE_FAILED =
        CEncodeError(CEncodeError::FailedWritingEncoded, u"Failed writing the encoded image(s)."_s);

    // Encoding
    static inline const QString OUTPUT_EXT = u"png"_s;

    // Messages - All
    static inline const QString MSG_COMMAND_INVOCATION = PROJECT_SHORT_NAME u" Encode\n--------------"_s;
    static inline const QString MSG_BPC = u"Bits per channel: %1"_s;
    static inline const QString MSG_PAYLOAD_SIZE = u"Payload size: %1"_s;
    static inline const QString MSG_ENCODING = u"Encoding: %1"_s;
    static inline const QString MSG_START_ENCODING = u"Encoding image..."_s;

    // Messages - Single
    static inline const QString MSG_SINGLE_MEDIUM_DIM = u"Medium dimensions: %1 x %2"_s;
    static inline const QString MSG_SINGLE_ACTUAL_BPC = u"Final bits per channel: %1"_s;
    static inline const QString MSG_SINGLE_IMAGE_SAVED = u"Wrote encoded image to '%1'"_s;

    // Messages - Multi
    static inline const QString MSG_MULTI_IMAGE_COUNT = u"Using %1 images"_s;
    static inline const QString MSG_MULTI_APROX_BPC = u"Final greatest bits per channel: %1"_s;
    static inline const QString MSG_MULTI_IMAGE_SAVED = u"Wrote encoded images to '%1'"_s;

    // Command line option strings
    static inline const QString CL_OPT_INPUT_S_NAME = u"i"_s;
    static inline const QString CL_OPT_INPUT_L_NAME = u"input"_s;
    static inline const QString CL_OPT_INPUT_DESC = u"Path to input file for encoding."_s;

    static inline const QString CL_OPT_OUTPUT_S_NAME = u"o"_s;
    static inline const QString CL_OPT_OUTPUT_L_NAME = u"output"_s;
    static inline const QString CL_OPT_OUTPUT_DESC = u"Path to encoded output file or directory. Defaults to the input path with a '_enc' suffix."_s;

    static inline const QString CL_OPT_MEDIUM_S_NAME = u"m"_s;
    static inline const QString CL_OPT_MEDIUM_L_NAME = u"medium"_s;
    static inline const QString CL_OPT_MEDIUM_DESC = u"The image(s) to encode the data within. A path to a directory performs a multi-part encode with all contained images."_s;

    static inline const QString CL_OPT_DENSITY_S_NAME = u"d"_s;
    static inline const QString CL_OPT_DENSITY_L_NAME = u"density"_s;
    static inline const QString CL_OPT_DENSITY_DESC = u"How many bits-per-channel to use when encoding the image (auto | 1-7). "
                                                      "Defaults to 'auto'"_s;
    static inline const QString CL_OPT_DENSITY_DEFAULT = u"auto"_s;

    static inline const QString CL_OPT_KEY_S_NAME = u"k"_s;
    static inline const QString CL_OPT_KEY_L_NAME = u"key"_s;
    static inline const QString CL_OPT_KEY_DESC = u"An optional key to require in order to decode the encoded image."_s;
    static inline const QString CL_OPT_KEY_DEFAULT = u""_s;

    static inline const QString CL_OPT_ENCODING_S_NAME = u"e"_s;
    static inline const QString CL_OPT_ENCODING_L_NAME = u"encoding"_s;
    static inline const QString CL_OPT_ENCODING_DESC =
        u"The type of encoding to use (defaults to Absolute):\n"
        "\n"
        "Relative - Requires the original medium(s) to decode\n"
        "Absolute - Doesn't require the original medium(s) to decode\n"_s;
    static inline const QString CL_OPT_ENCODING_DEFAULT = u"Absolute"_s;

    /* NOTE: This will cause a compilation error when changing PxCrypt::Encoder::Encoding in order to prompt the developer
     * to ensure any new options have been described above and then manually check them off here
     */
    static_assert(magic_enum::enum_values<PxCrypt::Encoder::Encoding>() == std::array<PxCrypt::Encoder::Encoding, 2>{
            PxCrypt::Encoder::Encoding::Relative,
            PxCrypt::Encoder::Encoding::Absolute
        },
        "Missing description for an encoding type"
    );

    // Command line options
    static inline const QCommandLineOption CL_OPTION_INPUT{{CL_OPT_INPUT_S_NAME, CL_OPT_INPUT_L_NAME}, CL_OPT_INPUT_DESC, "input"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_OUTPUT{{CL_OPT_OUTPUT_S_NAME, CL_OPT_OUTPUT_L_NAME}, CL_OPT_OUTPUT_DESC, "output"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_MEDIUM{{CL_OPT_MEDIUM_S_NAME, CL_OPT_MEDIUM_L_NAME}, CL_OPT_MEDIUM_DESC, "medium"}; // Takes value
    static inline const QCommandLineOption CL_OPTION_DENSITY{{CL_OPT_DENSITY_S_NAME, CL_OPT_DENSITY_L_NAME}, CL_OPT_DENSITY_DESC, "density", CL_OPT_DENSITY_DEFAULT}; // Takes value
    static inline const QCommandLineOption CL_OPTION_KEY{{CL_OPT_KEY_S_NAME, CL_OPT_KEY_L_NAME}, CL_OPT_KEY_DESC, "key", CL_OPT_KEY_DEFAULT}; // Takes value
    static inline const QCommandLineOption CL_OPTION_TYPE{{CL_OPT_ENCODING_S_NAME, CL_OPT_ENCODING_L_NAME}, CL_OPT_ENCODING_DESC, "encoding", CL_OPT_ENCODING_DEFAULT}; // Takes value

    static inline const QList<const QCommandLineOption*> CL_OPTIONS_SPECIFIC{&CL_OPTION_INPUT, &CL_OPTION_OUTPUT, &CL_OPTION_MEDIUM,
                                                                             &CL_OPTION_DENSITY, &CL_OPTION_KEY, &CL_OPTION_TYPE};
    static inline const QSet<const QCommandLineOption*> CL_OPTIONS_REQUIRED{&CL_OPTION_INPUT, &CL_OPTION_MEDIUM};

public:
    // Meta
    static inline const QString NAME = u"encode"_s;
    static inline const QString DESCRIPTION = u"Store a file within the color channel data of an image or images."_s;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    CEncode(Core& coreRef);

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    Qx::Error encodeSingleImage(const Job& job);
    Qx::Error encodeMultipleImages(const Job& job);

protected:
    const QList<const QCommandLineOption*> options() override;
    const QSet<const QCommandLineOption*> requiredOptions() override;
    const QString name() override;

public:
    Qx::Error process(const QStringList& commandLine) override;
};
REGISTER_COMMAND(CEncode::NAME, CEncode, CEncode::DESCRIPTION);

#endif // CENCODE_H
