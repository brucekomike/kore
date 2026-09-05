#include "ProjectScanner.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

namespace {

// Walks `path` recursively, skipping the contents of any `.git` directory
// (which can be large and are not relevant to the user-visible size/activity
// of a project), accumulating the total file size and the most recent
// modification time seen.
void walkDirectory(const QString &path, qint64 &totalSize, QDateTime &latestModified)
{
    QDir dir(path);
    const QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            if (entry.fileName() == QLatin1String(".git")) {
                continue;
            }
            walkDirectory(entry.absoluteFilePath(), totalSize, latestModified);
            continue;
        }
        totalSize += entry.size();
        const QDateTime modified = entry.lastModified();
        if (modified.isValid() && modified > latestModified) {
            latestModified = modified;
        }
    }
}

} // namespace

bool ProjectScanner::isGitRepository(const QString &path)
{
    return QFileInfo::exists(path + QStringLiteral("/.git"));
}

qint64 ProjectScanner::directorySize(const QString &path)
{
    qint64 size = 0;
    QDateTime lastModified;
    scanTree(path, size, lastModified);
    return size;
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
    qint64 size = 0;
    QDateTime lastModified;
    scanTree(path, size, lastModified);
    return lastModified;
}

void ProjectScanner::scanTree(const QString &path, qint64 &size, QDateTime &lastModified)
{
    size = 0;
    lastModified = QFileInfo(path).lastModified();
    walkDirectory(path, size, lastModified);
}

ProjectInfo ProjectScanner::makeProjectInfo(const QString &path, const QString &group)
{
    ProjectInfo info;
    info.name = QFileInfo(path).fileName();
    info.path = path;
    info.group = group;
    info.isGitRepo = isGitRepository(path);

    // A single filesystem walk computes both the size and last-modified
    // time, avoiding walking large project trees twice.
    qint64 size = 0;
    QDateTime lastModified;
    scanTree(path, size, lastModified);
    info.sizeBytes = size;

    info.lastUpdateTime = info.isGitRepo ? lastCommitTime(path) : QDateTime();
    if (!info.lastUpdateTime.isValid()) {
        info.lastUpdateTime = lastModified;
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
