#include "AppSettings.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QVariant>

namespace {
const char *kWorkspaceKey = "workspace/override";
const char *kIdeCommandKey = "commands/ide";
const char *kTerminalCommandKey = "commands/terminal";
const char *kOpenCodeCommandKey = "commands/opencode";
const char *kRecentKey = "recent/entries";
const int kMaxStoredRecent = 50;
} // namespace

AppSettings::AppSettings()
    : m_settings(QStringLiteral("kore"), QStringLiteral("kore"))
{
}

QString AppSettings::workspaceOverride() const
{
    return m_settings.value(QLatin1String(kWorkspaceKey)).toString();
}

void AppSettings::setWorkspaceOverride(const QString &path)
{
    m_settings.setValue(QLatin1String(kWorkspaceKey), path);
}

QString AppSettings::ideCommand() const
{
    // %1 is substituted with a shell-quoted project path by MainWindow, so
    // the template itself should not add its own quoting around %1.
    const QString configured = m_settings.value(QLatin1String(kIdeCommandKey)).toString();
    return configured.isEmpty() ? autoDetectIdeCommand() : configured;
}

void AppSettings::setIdeCommand(const QString &command)
{
    m_settings.setValue(QLatin1String(kIdeCommandKey), command);
}

QString AppSettings::autoDetectIdeCommand()
{
    const QStringList detected = detectedIdeCommands();
    return detected.isEmpty() ? QStringLiteral("code %1") : detected.first();
}

QStringList AppSettings::detectedIdeCommands()
{
    QStringList detected;
    const QStringList candidates = {
        QStringLiteral("zed"),
        QStringLiteral("code"),
        QStringLiteral("cursor"),
        QStringLiteral("windsurf")
    };
    for (const QString &candidate : candidates) {
        if (!QStandardPaths::findExecutable(candidate).isEmpty()) {
            detected.append(candidate + QStringLiteral(" %1"));
        }
    }
#if defined(Q_OS_MAC)
    const QList<QPair<QString, QString>> applications = {
        {QStringLiteral("Zed"), QStringLiteral("zed")},
        {QStringLiteral("Visual Studio Code"), QStringLiteral("code")},
        {QStringLiteral("Cursor"), QStringLiteral("cursor")},
    };
    for (const auto &application : applications) {
        if (QFileInfo::exists(QStringLiteral("/Applications/%1.app").arg(application.first))) {
            const QString command = QStringLiteral("open -a \"%1\" %2").arg(application.first, QStringLiteral("%1"));
            if (!detected.contains(command)) {
                detected.append(command);
            }
        }
    }
#endif
    return detected;
}

QString AppSettings::terminalCommand() const
{
    const QString configured = m_settings.value(QLatin1String(kTerminalCommandKey)).toString();
    return configured.isEmpty() ? autoDetectTerminalCommand() : configured;
}

void AppSettings::setTerminalCommand(const QString &command)
{
    m_settings.setValue(QLatin1String(kTerminalCommandKey), command);
}

QString AppSettings::autoDetectTerminalCommand()
{
    const QStringList detected = detectedTerminalCommands();
    return detected.isEmpty() ? defaultTerminalCommand() : detected.first();
}

QStringList AppSettings::detectedTerminalCommands()
{
    QStringList detected;
#if defined(Q_OS_WIN)
    if (!QStandardPaths::findExecutable(QStringLiteral("wt")).isEmpty()) {
        detected.append(QStringLiteral("wt -d %1"));
    }
    detected.append(QStringLiteral("cmd /K cd /d %1"));
#elif defined(Q_OS_MAC)
    if (QFileInfo::exists(QStringLiteral("/Applications/iTerm.app"))) {
        detected.append(QStringLiteral("open -a iTerm %1"));
    }
    detected.append(QStringLiteral("open -a Terminal %1"));
#else
    const QList<QString> terminals = {
        QStringLiteral("x-terminal-emulator"),
        QStringLiteral("gnome-terminal"),
        QStringLiteral("konsole"),
        QStringLiteral("kitty")
    };
    for (const QString &terminal : terminals) {
        if (!QStandardPaths::findExecutable(terminal).isEmpty()) {
            if (terminal == QStringLiteral("gnome-terminal")) {
                detected.append(terminal + QStringLiteral(" --working-directory=%1"));
                continue;
            }
            if (terminal == QStringLiteral("konsole")) {
                detected.append(terminal + QStringLiteral(" --workdir %1"));
                continue;
            }
            if (terminal == QStringLiteral("kitty")) {
                detected.append(terminal + QStringLiteral(" --directory %1"));
                continue;
            }
            detected.append(terminal + QStringLiteral(" --working-directory=%1"));
        }
    }
#endif
    return detected;
}

QString AppSettings::openCodeCommand() const
{
    const QString configured = m_settings.value(QLatin1String(kOpenCodeCommandKey)).toString();
    return configured.isEmpty() ? autoDetectAgentIdeCommand() : configured;
}

void AppSettings::setOpenCodeCommand(const QString &command)
{
    m_settings.setValue(QLatin1String(kOpenCodeCommandKey), command);
}

QString AppSettings::autoDetectAgentIdeCommand()
{
    const QStringList detected = detectedAgentIdeCommands();
    return detected.isEmpty() ? QStringLiteral("opencode %1") : detected.first();
}

QStringList AppSettings::detectedAgentIdeCommands()
{
    QStringList detected;
    if (!QStandardPaths::findExecutable(QStringLiteral("opencode")).isEmpty()) {
        detected.append(QStringLiteral("opencode %1"));
    }
#if defined(Q_OS_MAC)
    if (QFileInfo::exists(QStringLiteral("/Applications/OpenCode.app"))) {
        detected.append(QStringLiteral("open -a OpenCode %1"));
    }
#endif
    return detected;
}

QString AppSettings::defaultTerminalCommand()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("cmd /K cd /d %1");
#elif defined(Q_OS_MAC)
    return QStringLiteral("open -a Terminal %1");
#else
    return QStringLiteral("x-terminal-emulator --working-directory=%1");
#endif
}

void AppSettings::recordOpened(const QString &path)
{
    QVariantList entries = m_settings.value(QLatin1String(kRecentKey)).toList();

    // Remove any existing entry for this path.
    for (int i = entries.size() - 1; i >= 0; --i) {
        if (entries.at(i).toMap().value(QStringLiteral("path")).toString() == path) {
            entries.removeAt(i);
        }
    }

    QVariantMap entry;
    entry.insert(QStringLiteral("path"), path);
    entry.insert(QStringLiteral("time"), QDateTime::currentDateTime().toString(Qt::ISODate));
    entries.prepend(entry);

    while (entries.size() > kMaxStoredRecent) {
        entries.removeLast();
    }

    m_settings.setValue(QLatin1String(kRecentKey), entries);
}

QDateTime AppSettings::lastOpenTime(const QString &path) const
{
    const QVariantList entries = m_settings.value(QLatin1String(kRecentKey)).toList();
    for (const QVariant &entry : entries) {
        const QVariantMap map = entry.toMap();
        if (map.value(QStringLiteral("path")).toString() == path) {
            return QDateTime::fromString(map.value(QStringLiteral("time")).toString(), Qt::ISODate);
        }
    }
    return QDateTime();
}

QStringList AppSettings::recentProjects(int maxCount) const
{
    QStringList paths;
    if (maxCount <= 0) {
        return paths;
    }

    const QVariantList entries = m_settings.value(QLatin1String(kRecentKey)).toList();
    for (const QVariant &entry : entries) {
        paths.append(entry.toMap().value(QStringLiteral("path")).toString());
        if (paths.size() >= maxCount) {
            break;
        }
    }
    return paths;
}

void AppSettings::removeRecentProject(const QString &path)
{
    QVariantList entries = m_settings.value(QLatin1String(kRecentKey)).toList();
    for (int i = entries.size() - 1; i >= 0; --i) {
        if (entries.at(i).toMap().value(QStringLiteral("path")).toString() == path) {
            entries.removeAt(i);
        }
    }
    m_settings.setValue(QLatin1String(kRecentKey), entries);
}

void AppSettings::clearRecentProjects()
{
    m_settings.remove(QLatin1String(kRecentKey));
}
