#include "WorkspaceResolver.h"

#include <QDir>
#include <QStandardPaths>

QStringList WorkspaceResolver::candidateNames()
{
    return {QStringLiteral("Workspace"), QStringLiteral("Projects")};
}

QString WorkspaceResolver::resolve(const QString &override)
{
    if (!override.isEmpty()) {
        QDir dir(override);
        if (dir.exists()) {
            return dir.absolutePath();
        }
    }

    const QString home = QDir::homePath();
    for (const QString &name : candidateNames()) {
        QDir dir(home + QLatin1Char('/') + name);
        if (dir.exists()) {
            return dir.absolutePath();
        }
    }

    // Nothing found: fall back to the home directory so the app still has a
    // valid, existing folder to display.
    return home;
}
