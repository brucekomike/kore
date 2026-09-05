#pragma once

#include <QDateTime>
#include <QSettings>
#include <QString>
#include <QStringList>

// Persists user configuration and usage history for kore using QSettings
// (native platform storage: registry on Windows, plist on macOS, ini on
// Linux).
class AppSettings
{
public:
    AppSettings();

    // Workspace root override. Empty means "auto-resolve".
    QString workspaceOverride() const;
    void setWorkspaceOverride(const QString &path);

    // Command used to open a project in an IDE. `%1` is replaced with the
    // project path.
    QString ideCommand() const;
    void setIdeCommand(const QString &command);
    static QStringList detectedIdeCommands();
    static QString autoDetectIdeCommand();

    // Command used to open a project in a CLI. `%1` is replaced with
    // the project path.
    QString terminalCommand() const;
    void setTerminalCommand(const QString &command);
    static QStringList detectedTerminalCommands();
    static QString autoDetectTerminalCommand();

    // Command used to open a project in an agent IDE. `%1` is replaced
    // with the project path.
    QString openCodeCommand() const;
    void setOpenCodeCommand(const QString &command);
    static QStringList detectedAgentIdeCommands();
    static QString autoDetectAgentIdeCommand();

    // Records that `path` was just opened, updating its last-opened
    // timestamp and moving it to the front of the recent list.
    void recordOpened(const QString &path);

    // Returns the last time `path` was opened, or an invalid QDateTime if
    // it has never been opened.
    QDateTime lastOpenTime(const QString &path) const;

    // Returns the list of recently opened project paths, most recent first.
    QStringList recentProjects(int maxCount = 10) const;

    // Platform-appropriate default command for opening a CLI.
    static QString defaultTerminalCommand();

private:
    QSettings m_settings;
};
