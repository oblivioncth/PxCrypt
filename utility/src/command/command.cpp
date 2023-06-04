// Unit Includes
#include "command.h"

// Qx Includes
#include <qx/utility/qx-helpers.h>

//===============================================================================================================
// Command
//===============================================================================================================

//-Constructor----------------------------------------------------------------------------------------------------------
//Public:
Command::Command(Core& coreRef) : mCore(coreRef) {}

//-Class Functions------------------------------------------------------------------
//Private:
QMap<QString, Command::Entry>& Command::registry() { static QMap<QString, Entry> registry; return registry; }

//Public:
void Command::registerCommand(const QString& name, CommandFactory* factory, const QString& desc) { registry()[name] = {factory, desc}; }
bool Command::isRegistered(const QString &name) { return registry().contains(name); }
QList<QString> Command::registered() { return registry().keys(); }
QString Command::describe(const QString& name) { return registry().value(name).description; }
std::unique_ptr<Command> Command::acquire(const QString& name, Core& coreRef) { return registry().value(name).factory->produce(coreRef); }

//-Instance Functions------------------------------------------------------------------------------------------------------
//Protected:
const QList<const QCommandLineOption*> Command::options() { return CL_OPTIONS_STANDARD; }

ErrorCode Command::parse(const QStringList& commandLine)
{
    // Add command options
    for(const QCommandLineOption* clOption : options())
        mParser.addOption(*clOption);

    // Parse
    if(mParser.parse(commandLine))
        return ErrorCode::NO_ERR;
    else
    {
        mCore.printError(NAME, Qx::GenericError(Qx::GenericError::Error, Core::ERR_INVALID_PARAM, mParser.errorText()));
        return ErrorCode::INVALID_ARGS;
    }
}

bool Command::checkStandardOptions()
{
    // Show help if requested
    if(mParser.isSet(CL_OPTION_HELP))
    {
        showHelp();
        return true;
    }

    // Default
    return false;
}

Qx::GenericError Command::checkRequiredOptions()
{
    QStringList missing;
    for(auto opt : qxAsConst(requiredOptions()))
        if(mParser.isSet(*opt))
            missing << opt->names().constFirst();

    if(!missing.isEmpty())
        return Qx::GenericError(Qx::GenericError::Error, ERR_MISSING_REQ_OPT.arg(name()), "'" + missing.join("','") + "'");

    return Qx::GenericError();
}

void Command::showHelp()
{
    // Help string
    static QString helpStr;

    // One time setup
    if(helpStr.isNull())
    {
        // Help options
        QString optStr;
        for(const QCommandLineOption* clOption : options())
        {
            // Handle names
            QStringList dashedNames;
            for(const QString& name :  qxAsConst(clOption->names()))
                dashedNames << ((name.length() > 1 ? "--" : "-") + name);

            // Add option
            QString marker = requiredOptions().contains(clOption) ? "*" : "";
            optStr += HELP_OPT_TEMPL.arg(marker, dashedNames.join(" | "), clOption->description());
        }

        // Complete string
        helpStr = HELP_TEMPL.arg(name(), optStr);
    }

    // Show help
    mCore.printMessage(NAME, helpStr);
}
