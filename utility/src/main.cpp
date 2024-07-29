// Qt Includes
#include <QCoreApplication>

// Qx Includes
#include <qx/core/qx-iostream.h>

// Project Includes
#include "kernel/core.h"
#include "command/command.h"
#include "project_vars.h"

// BUG: THERE IS A FLAW IN THE UTILITY WHERE BAD SWITCHES SEEM TO CAUSE NO OUTPUT

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
    CoreError initError = core.initialize(command, commandParam);
    if(initError.isValid())
        return Qx::Error(initError).code();

    // Terminate if no command
    if(command.isEmpty())
        return 0;

    // Check for valid command
    CommandError commandCheckErr = Command::isRegistered(command);
    if(commandCheckErr.isValid())
    {
        core.printError(NAME, commandCheckErr);
        return Qx::Error(commandCheckErr).code();
    }

    // Create command instance
    std::unique_ptr<Command> commandProcessor = Command::acquire(command, core);

    // Process command
    return commandProcessor->process(commandParam).code();
}
