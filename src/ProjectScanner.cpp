#include "ProjectScanner.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>

bool ProjectScanner::isGitRepository(const QString &path)
{
    return QFileInfo::exists(path + QStringLiteral("/.git"));
}

qint64 ProjectScanner::directorySize(const QString &path)
{
    qint64 total = 0;
    QDirIterator it(path, QDir::Files | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        total += it.fileInfo().size();
    }
    return total;
}

QDateTime ProjectScanner::lastCommitTime(const QString &path)
{
    QProcess git;
    git.setWorkingDirectory(path);
    git.start(QStringLiteral("git"), {QStringLiteral("log"), QStringLiteral("-1"), QStringLiteral("--format=%cI")});
    if (!git.waitForStarted(2000) || !git.waitForFinished(3000)) {
        return QDateTime();
    }
    if (git.exitStatus() != QProcess::NormalExit || git.exitCode() != 0) {
        return QDateTime();
    }
    const QString output = QString::fromUtf8(git.readAllStandardOutput()).trimmed();
    if (output.isEmpty()) {
        return QDateTime();
    }
    return QDateTime::fromString(output, Qt::ISODate);
}

QDateTime ProjectScanner::lastModifiedRecursive(const QString &path)
{
    QDateTime latest = QFileInfo(path).lastModified();
    QDirIterator it(path, QDir::Files | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QDateTime modified = it.fileInfo().lastModified();
        if (modified.isValid() && modified > latest) {
            latest = modified;
        }
    }
    return latest;
}

ProjectInfo ProjectScanner::makeProjectInfo(const QString &path, const QString &group)
{
    ProjectInfo info;
    info.name = QFileInfo(path).fileName();
    info.path = path;
    info.group = group;
    info.isGitRepo = isGitRepository(path);
    info.sizeBytes = directorySize(path);
    info.lastUpdateTime = info.isGitRepo ? lastCommitTime(path) : QDateTime();
    if (!info.lastUpdateTime.isValid()) {
        info.lastUpdateTime = lastModifiedRecursive(path);
    }
    return info;
}

ProjectList ProjectScanner::scan(const QString &workspaceRoot)
{
    ProjectList projects;

    QDir root(workspaceRoot);
    if (!root.exists()) {
        return projects;
    }

    const QFileInfoList topEntries = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &entry : topEntries) {
        const QString entryPath = entry.absoluteFilePath();
        if (isGitRepository(entryPath)) {
            // A git repository directly under the workspace root: ungrouped project.
            projects.append(makeProjectInfo(entryPath, QString()));
            continue;
        }

        // Not a repository itself: treat it as a grouping folder and look one
        // level deeper for the actual projects.
        QDir groupDir(entryPath);
        const QFileInfoList children = groupDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        if (children.isEmpty()) {
            continue;
        }
        for (const QFileInfo &child : children) {
            projects.append(makeProjectInfo(child.absoluteFilePath(), entry.fileName()));
        }
    }

    return projects;
}
