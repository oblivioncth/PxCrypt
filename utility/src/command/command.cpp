// Unit Includes
#include "command.h"

// Qx Includes
#include <qx/utility/qx-helpers.h>
#include <qx/core/qx-string.h>

//===============================================================================================================
// CommandError
//===============================================================================================================

//-Constructor-----------------------------------------------------------------------------------------------------
//Public:
CommandError::CommandError() :
    mType(NoError)
{}

//Private:
CommandError::CommandError(Type type, const QString& errStr) :
    mType(type),
    mString(errStr)
{}

//-Instance Functions----------------------------------------------------------------------------------------------
//Private:
quint32 CommandError::deriveValue() const { return static_cast<quint32>(mType); }
QString CommandError::derivePrimary() const { return PRIMARY_STRING; }
QString CommandError::deriveSecondary() const { return mString; }
QString CommandError::deriveDetails() const { return mDetails; }
Qx::Severity CommandError::deriveSeverity() const { return Qx::Critical; }
CommandError& CommandError::wDetails(const QString& det) { mDetails = det; return *this; }

//Public:
bool CommandError::isValid() const { return mType != NoError; }
CommandError::Type CommandError::type() const { return mType; }
QString CommandError::errorString() const { return mString; }

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

CommandError Command::isRegistered(const QString &name)
{
    return registry().contains(name) ? CommandError() : ERR_INVALID_COMMAND.arged(name);
}

QList<QString> Command::registered() { return registry().keys(); }
QString Command::describe(const QString& name) { return registry().value(name).description; }
std::unique_ptr<Command> Command::acquire(const QString& name, Core& coreRef) { return registry().value(name).factory->produce(coreRef); }

//-Instance Functions------------------------------------------------------------------------------------------------------
//Private:
CommandError Command::parse(const QStringList& commandLine)
{
    // Add command options
    for(const QCommandLineOption* clOption : options())
        mParser.addOption(*clOption);

    // Parse
    if(mParser.parse(commandLine))
        return CommandError();
    else
        return CommandError(ERR_INVALID_ARGS).wDetails(mParser.errorText());
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

CommandError Command::checkRequiredOptions()
{
    QStringList missing;
    for(auto opt : qxAsConst(requiredOptions()))
        if(!mParser.isSet(*opt))
            missing.append(opt->names().constFirst());

    if(!missing.isEmpty())
        return ERR_MISSING_REQ_OPT.arged(name()).wDetails("'" + missing.join("','") + "'");

    return CommandError();
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

//Protected:
const QList<const QCommandLineOption*> Command::options() { return CL_OPTIONS_STANDARD; }

//Public:
Qx::Error Command::process(const QStringList& commandLine)
{
    // Print invocation text
    QString inv = PROJECT_SHORT_NAME u" "_s + Qx::String::toHeadlineCase(name()) + '\n';
    inv.append(u"-"_s.repeated(inv.size() - 1)); // -1 for '\n'
    mCore.printMessage(NAME, inv);

    // Parse and check for valid arguments
    if(CommandError parseError = parse(commandLine))
    {
        mCore.printError(NAME, parseError);
        return parseError;
    }

    // Handle standard options
    if(checkStandardOptions())
        return CommandError();

    // Check for required options
    if(CommandError reqCheck = checkRequiredOptions())
    {
        mCore.printError(NAME, reqCheck);
        return reqCheck;
    }

    return perform();
}
