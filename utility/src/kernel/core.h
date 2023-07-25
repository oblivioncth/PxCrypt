#ifndef CORE_H
#define CORE_H

// Qt Includes
#include <QCommandLineParser>

// Qx Includes
#include <qx/core/qx-error.h>

// Project Includes
#include "project_vars.h"

#define ENUM_NAME(eenum) QString(magic_enum::enum_name(eenum).data())

class QX_ERROR_TYPE(CoreError, "CoreError", 2000)
{
    friend class Core;

//-Class Enums--------------------------------------------------------------------------------------------------------
public:
    enum Type
    {
        NoError = 0,
        InvalidArguments = 1
    };

//-Instance Variables------------------------------------------------------------------------------------------------
private:
    Type mType;
    QString mGeneral;
    QString mSpecific;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    CoreError();

private:
    CoreError(Type type, const QString& gen);

//-Instance Functions---------------------------------------------------------------------------------------------------
private:
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    Qx::Severity deriveSeverity() const override;

    CoreError wSpecific(const QString& spec) const;

public:
    bool isValid() const;
    Type type() const;
    QString errorString() const;
};

class Core
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Error
    static inline const CoreError ERR_INVALID_ARGS =
            CoreError(CoreError::InvalidArguments, "Invalid arguments.");

    // Global command line option strings
    static inline const QString CL_OPT_HELP_S_NAME = u"h"_s;
    static inline const QString CL_OPT_HELP_L_NAME = u"help"_s;
    static inline const QString CL_OPT_HELP_E_NAME = u"?"_s;
    static inline const QString CL_OPT_HELP_DESC = u"Prints this help message."_s;

    static inline const QString CL_OPT_VERSION_S_NAME = u"v"_s;
    static inline const QString CL_OPT_VERSION_L_NAME = u"version"_s;
    static inline const QString CL_OPT_VERSION_DESC = u"Prints the current version of this tool."_s;

    static inline const QString CL_OPT_FORMATS_S_NAME = u"f"_s;
    static inline const QString CL_OPT_FORMATS_L_NAME = u"formats"_s;
    static inline const QString CL_OPT_FORMATS_DESC = u"Prints the image formats this tool supports."_s;

    // Global command line options
    static inline const QCommandLineOption CL_OPTION_HELP{{CL_OPT_HELP_S_NAME, CL_OPT_HELP_L_NAME, CL_OPT_HELP_E_NAME}, CL_OPT_HELP_DESC}; // Boolean option
    static inline const QCommandLineOption CL_OPTION_VERSION{{CL_OPT_VERSION_S_NAME, CL_OPT_VERSION_L_NAME}, CL_OPT_VERSION_DESC}; // Boolean option
    static inline const QCommandLineOption CL_OPTION_FORMATS{{CL_OPT_FORMATS_S_NAME, CL_OPT_FORMATS_L_NAME}, CL_OPT_FORMATS_DESC}; // Boolean option

    static inline const QList<const QCommandLineOption*> CL_OPTIONS_ALL{&CL_OPTION_HELP, &CL_OPTION_VERSION, &CL_OPTION_FORMATS};

    // Help template
    static inline const QString HELP_TEMPL = u"Usage:\n"
                                             "------\n"
                                             PROJECT_SHORT_NAME " <global options> [command] <command options>\n"
                                             "\n"
                                             "Global Options:\n"
                                             "---------------%1\n"
                                             "\n"
                                             "Commands:\n"
                                             "---------%2\n"
                                             "\n"
                                             "Use the '-h' switch after a command to see it's specific usage notes\n"_s;
    static inline const QString HELP_OPT_TEMPL = u"\n%1: %2"_s;
    static inline const QString HELP_COMMAND_TEMPL = u"\n[%1]: %2"_s;

    // Messages
    static inline const QString MSG_VERSION = PROJECT_SHORT_NAME u" version "_s PROJECT_VERSION_STR u"\n"_s;
    static inline const QString MSG_FORMATS = PROJECT_SHORT_NAME u" supports the following image formats as input:\n%1\n"_s;

    // Meta
    static inline const QString NAME = u"Core"_s;

public:
    // Error Messages
    static inline const QString ERR_INVALID_PARAM = u"Invalid arguments. Use " PROJECT_SHORT_NAME " -h for help"_s;

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QStringList mImageFormats;
    QStringList mImageFormatFilter;

    // Processing
    const QStringList mArguments;

//-Constructor----------------------------------------------------------------------------------------------------------
public:
    Core(QCoreApplication* app);

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    void showHelp();
    void showVersion();
    void showFormats();

public:
    CoreError initialize(QString& command, QStringList& commandParam);

    QStringList imageFormatFilter() const;
    QStringList supportedImageFormats() const;

    void printError(QString src, Qx::Error error);
    void printMessage(QString src, QString msg);
};

#endif // CORE_H
