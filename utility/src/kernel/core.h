#ifndef CORE_H
#define CORE_H

// Qt Includes
#include <QCommandLineParser>

// Qx Includes
#include <qx/core/qx-genericerror.h>
#include <qx/utility/qx-macros.h>

// Project Includes
#include "errorcode.h"
#include "project_vars.h"

#define ENUM_NAME(eenum) QString(magic_enum::enum_name(eenum).data())

class Core
{
//-Class Variables------------------------------------------------------------------------------------------------------
private:
    // Global command line option strings
    static inline const QString CL_OPT_HELP_S_NAME = QSL("h");
    static inline const QString CL_OPT_HELP_L_NAME = QSL("help");
    static inline const QString CL_OPT_HELP_E_NAME = QSL("?");
    static inline const QString CL_OPT_HELP_DESC = QSL("Prints this help message.");

    static inline const QString CL_OPT_VERSION_S_NAME = QSL("v");
    static inline const QString CL_OPT_VERSION_L_NAME = QSL("version");
    static inline const QString CL_OPT_VERSION_DESC = QSL("Prints the current version of this tool.");

    static inline const QString CL_OPT_FORMATS_S_NAME = QSL("f");
    static inline const QString CL_OPT_FORMATS_L_NAME = QSL("formats");
    static inline const QString CL_OPT_FORMATS_DESC = QSL("Prints the image formats this tool supports.");

    // Global command line options
    static inline const QCommandLineOption CL_OPTION_HELP{{CL_OPT_HELP_S_NAME, CL_OPT_HELP_L_NAME, CL_OPT_HELP_E_NAME}, CL_OPT_HELP_DESC}; // Boolean option
    static inline const QCommandLineOption CL_OPTION_VERSION{{CL_OPT_VERSION_S_NAME, CL_OPT_VERSION_L_NAME}, CL_OPT_VERSION_DESC}; // Boolean option
    static inline const QCommandLineOption CL_OPTION_FORMATS{{CL_OPT_FORMATS_S_NAME, CL_OPT_FORMATS_L_NAME}, CL_OPT_FORMATS_DESC}; // Boolean option

    static inline const QList<const QCommandLineOption*> CL_OPTIONS_ALL{&CL_OPTION_HELP, &CL_OPTION_VERSION, &CL_OPTION_FORMATS};

    // Help template
    static inline const QString HELP_TEMPL = QSL("Usage:\n"
                                                 "------\n"
                                                 PROJECT_SHORT_NAME " <global options> [command] <command options>\n"
                                                 "\n"
                                                 "Global Options:\n"
                                                 "---------------%1\n"
                                                 "\n"
                                                 "Commands:\n"
                                                 "---------%2\n"
                                                 "\n"
                                                 "Use the '-h' switch after a command to see it's specific usage notes\n");
    static inline const QString HELP_OPT_TEMPL = QSL("\n%1: %2");
    static inline const QString HELP_COMMAND_TEMPL = QSL("\n[%1]: %2");

    // Messages
    static inline const QString MSG_VERSION = QSL(PROJECT_SHORT_NAME " version " PROJECT_VERSION_STR "\n");
    static inline const QString MSG_FORMATS = QSL(PROJECT_SHORT_NAME " supports the following image formats:\n%1\n");

    // Meta
    static inline const QString NAME = QSL("Core");

public:
    // Error Messages
    static inline const QString ERR_INVALID_PARAM = QSL("Invalid arguments. Use " PROJECT_SHORT_NAME " -h for help");

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
    ErrorCode initialize(QString& command, QStringList& commandParam);

    QStringList imageFormatFilter() const;
    QStringList supportedImageFormats() const;

    void printError(QString src, Qx::GenericError error);
    void printMessage(QString src, QString msg);
};

#endif // CORE_H
