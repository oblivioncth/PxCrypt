#ifndef COMMAND_H
#define COMMAND_H

// Qx Includes
#include <qx/utility/qx-macros.h>

// Project Includes
#include "kernel/core.h"

class CommandFactory;

//-Macros-------------------------------------------------------------------------------------------------------------------
#define REGISTER_COMMAND(name, command, desc) \
    class command##Factory : public CommandFactory \
    { \
    public: \
        command##Factory() { Command::registerCommand(name, this, desc); } \
        virtual std::unique_ptr<Command> produce(Core& coreRef) { return std::make_unique<command>(coreRef); } \
    }; \
    static command##Factory _##command##Factory;


class Command
{
//-Class Structs------------------------------------------------------------------------------------------------------
protected:
    struct Entry
    {
        CommandFactory* factory;
        QString description;
    };

//-Class Variables--------------------------------------------------------------------------------------------------------
private:
    // Error
    static inline const QString ERR_MISSING_REQ_OPT = QSL("Missing required options for '%1'");

    // Help template
    static inline const QString HELP_TEMPL = QSL("Usage:\n"
                                             "------\n"
                                             "%1 <options>\n"
                                             "\n"
                                             "Options:\n"
                                             "--------%2\n"
                                             "\n"
                                             "*Required Option\n");
    static inline const QString HELP_OPT_TEMPL = QSL("\n%1%2: %3");

    // Standard command line option strings
    static inline const QString CL_OPT_HELP_S_NAME = QSL("h");
    static inline const QString CL_OPT_HELP_L_NAME = QSL("help");
    static inline const QString CL_OPT_HELP_E_NAME = QSL("?");
    static inline const QString CL_OPT_HELP_DESC = QSL("Prints this help message.");

    // Standard command line options
    static inline const QCommandLineOption CL_OPTION_HELP{{CL_OPT_HELP_S_NAME, CL_OPT_HELP_L_NAME, CL_OPT_HELP_E_NAME}, CL_OPT_HELP_DESC}; // Boolean option
    static inline const QList<const QCommandLineOption*> CL_OPTIONS_STANDARD{&CL_OPTION_HELP};

    // Meta
    static inline const QString NAME = QSL("command");

//-Instance Variables------------------------------------------------------------------------------------------------------
private:
    QString mHelpString;

protected:
    Core& mCore;
    QCommandLineParser mParser;

//-Constructor----------------------------------------------------------------------------------------------------------
protected:
    Command(Core& coreRef);

//-Destructor----------------------------------------------------------------------------------------------------------
public:
    virtual ~Command() = default;

//-Class Functions----------------------------------------------------------------------------------------------------------
private:
    static QMap<QString, Entry>& registry();

public:
    static void registerCommand(const QString& name, CommandFactory* factory, const QString& desc);
    static bool isRegistered(const QString& name);
    static QList<QString> registered();
    static std::unique_ptr<Command> acquire(const QString& name, Core& coreRef);
    static QString describe(const QString& name);

//-Instance Functions------------------------------------------------------------------------------------------------------
protected:
    virtual const QList<const QCommandLineOption*> options() = 0;
    virtual const QSet<const QCommandLineOption*> requiredOptions() = 0;
    virtual const QString name() = 0;
    ErrorCode parse(const QStringList& commandLine);
    bool checkStandardOptions();
    Qx::GenericError checkRequiredOptions();
    void showHelp();

public:
    virtual ErrorCode process(const QStringList& commandLine) = 0;
};

class CommandFactory
{
//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    virtual std::unique_ptr<Command> produce(Core& coreRef) = 0;
};

#endif // COMMAND_H
