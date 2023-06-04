// Unit Include
#include "core.h"

// Qt Includes
#include <QCoreApplication>
#include <QImageReader>

// Qx Includes
#include <qx/core/qx-iostream.h>
#include <qx/utility/qx-helpers.h>

// Project Includes
#include "command/command.h"

// Macros
#define ENUM_NAME(eenum) QString(magic_enum::enum_name(eenum).data())

//===============================================================================================================
// CORE
//===============================================================================================================

//-Constructor-------------------------------------------------------------
Core::Core(QCoreApplication* app) :
    mArguments(app->arguments())
{
    // Note supported formats
    for(const QByteArray& format : qxAsConst(QImageReader::supportedImageFormats()))
    {
        QString formatExt = QString::fromUtf8(format);
        mImageFormats.append(formatExt);
        mImageFormatFilter.append("*." + formatExt);
    }
}

//-Instance Functions-------------------------------------------------------------
//Private:
void Core::showHelp()
{
    // Help string
    static QString helpStr;

    // One time setup
    if(helpStr.isNull())
    {
        // Help options
        QString optStr;
        for(const QCommandLineOption* clOption : CL_OPTIONS_ALL)
        {
            // Handle names
            QStringList dashedNames;
            for(const QString& name : qxAsConst(clOption->names()))
                dashedNames << ((name.length() > 1 ? "--" : "-") + name);

            // Add option
            optStr += HELP_OPT_TEMPL.arg(dashedNames.join(" | "), clOption->description());
        }

        // Help commands
        QString commandStr;
        for(const QString& command : qxAsConst(Command::registered()))
            commandStr += HELP_COMMAND_TEMPL.arg(command, Command::describe(command));

        // Complete string
        helpStr = HELP_TEMPL.arg(optStr, commandStr);
    }

    // Show help
    printMessage(NAME, helpStr);
}

void Core::showVersion() { printMessage(NAME, MSG_VERSION); }
void Core::showFormats() { printMessage(NAME, MSG_FORMATS.arg(mImageFormats.join('\n'))); }

//Public:
ErrorCode Core::initialize(QString& command, QStringList& commandParam)
{
    // Clear return buffers
    command.clear();
    commandParam.clear();

    // Setup CLI Parser
    QCommandLineParser clParser;
    clParser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    for(const QCommandLineOption* clOption : CL_OPTIONS_ALL)
        clParser.addOption(*clOption);

    // Parse
    if(!clParser.parse(mArguments))
    {
        printError(NAME, Qx::GenericError(Qx::GenericError::Error, ERR_INVALID_PARAM, clParser.errorText()));
        showHelp();
        return ErrorCode::INVALID_ARGS;
    }

    QStringList cmdPart = clParser.positionalArguments();

    if(clParser.isSet(CL_OPTION_VERSION))
        showVersion();
    else if(clParser.isSet(CL_OPTION_FORMATS))
        showFormats();
    else if(clParser.isSet(CL_OPTION_HELP) || cmdPart.isEmpty()) // Also when no parameters
        showHelp();
    else
    {
        command = cmdPart.takeFirst();
        commandParam = cmdPart;
    }

    // Return success
    return ErrorCode::NO_ERR;
}

QStringList Core::imageFormatFilter() const { return mImageFormatFilter; }
QStringList Core::supportedImageFormats() const { return mImageFormats; }

void Core::printError(QString src, Qx::GenericError error)
{
    Q_UNUSED(src); // TODO: Incorporate
    Qx::cout << error << Qt::endl;
}

void Core::printMessage(QString src, QString msg)
{
    Q_UNUSED(src); // TODO: Incorporate
    Qx::cout << msg << Qt::endl;
}
