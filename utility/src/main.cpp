// Qt Includes
#include <QCoreApplication>

// Qx Includes
#include <qx/core/qx-iostream.h>

// Project Includes
#include "kernel/core.h"
#include "command/command.h"
#include "project_vars.h"

// Error Messages
const QString ERR_INVALID_COMMAND = R"("%1" is not a valid command)";

// Meta
const QString NAME = "main";

int main(int argc, char *argv[])
{
    // Setup application
    QCoreApplication app(argc, argv);
    app.setApplicationName(PROJECT_SHORT_NAME);
    app.setApplicationVersion(PROJECT_VERSION_STR);

    // Disable console input echo since this application isn't interactive (prevents accidental console clutter)
    Qx::setUserInputEchoEnabled(false);
    QScopeGuard inputEchoGuard([]{ Qx::setUserInputEchoEnabled(true); }); // Re-enable before app finishes

    // Create Core instance and initialize
    Core core(&app);
    QString command;
    QStringList commandParam;
    ErrorCode initError = core.initialize(command, commandParam);
    if(initError)
        return initError;

    // Terminate if no command
    if(command.isEmpty())
        return ErrorCode::NO_ERR;

    // Check for valid command
    if(!Command::isRegistered(command))
    {
        core.printError(NAME, Qx::GenericError(Qx::GenericError::Critical, ERR_INVALID_COMMAND.arg(command)));
        return ErrorCode::INVALID_ARGS;
    }

    // Create command instance
    std::unique_ptr<Command> commandProcessor = Command::acquire(command, core);

    // Process command
    return commandProcessor->process(commandParam);
}
