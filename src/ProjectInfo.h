#pragma once

#include <QDateTime>
#include <QList>
#include <QString>

// Basic information collected for a single project (repository) found under
// the workspace root.
struct ProjectInfo
{
    // Display name of the project (its directory name).
    QString name;

    // Absolute path to the project on disk.
    QString path;

    // Name of the immediate parent folder relative to the workspace root.
    // Used to group projects in the panel view. Empty when the project sits
    // directly under the workspace root.
    QString group;

    // Total size of the project directory on disk, in bytes.
    qint64 sizeBytes = 0;

    // Timestamp of the most recent change inside the project (last commit
    // time when the project is a git repository, otherwise the most recent
    // file modification time found on disk).
    QDateTime lastUpdateTime;

    // Timestamp the user last opened this project through kore. Invalid
    // when the project has never been opened.
    QDateTime lastOpenTime;

    // Whether the directory is a git repository (contains a .git entry).
    bool isGitRepo = false;
};

using ProjectList = QList<ProjectInfo>;
