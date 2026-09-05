#include "AppSettings.h"

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
    return m_settings.value(QLatin1String(kIdeCommandKey), QStringLiteral("code %1")).toString();
}

void AppSettings::setIdeCommand(const QString &command)
{
    m_settings.setValue(QLatin1String(kIdeCommandKey), command);
}

QString AppSettings::terminalCommand() const
{
    return m_settings.value(QLatin1String(kTerminalCommandKey), defaultTerminalCommand()).toString();
}

void AppSettings::setTerminalCommand(const QString &command)
{
    m_settings.setValue(QLatin1String(kTerminalCommandKey), command);
}

QString AppSettings::openCodeCommand() const
{
    return m_settings.value(QLatin1String(kOpenCodeCommandKey), QStringLiteral("opencode %1")).toString();
}

void AppSettings::setOpenCodeCommand(const QString &command)
{
    m_settings.setValue(QLatin1String(kOpenCodeCommandKey), command);
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
