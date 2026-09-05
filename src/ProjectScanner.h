#pragma once

#include "ProjectInfo.h"

#include <QString>

// Scans a workspace root directory for projects, optionally grouped by an
// intermediate subfolder, e.g.:
//
//   Workspace/
//     my-repo/            -> ungrouped project "my-repo"
//     work/
//       service-a/        -> project "service-a" grouped under "work"
//       service-b/        -> project "service-b" grouped under "work"
class ProjectScanner
{
public:
    // Scans `workspaceRoot` and returns the list of discovered projects.
    // A directory is considered a project if it is itself a git repository
    // (contains a `.git` entry), or if it is a plain directory found either
    // directly under the workspace root or one level below a grouping
    // subfolder.
    static ProjectList scan(const QString &workspaceRoot);

    // Returns true if `path` looks like a git repository.
    static bool isGitRepository(const QString &path);

    // Total size, in bytes, of all files contained in `path` (recursive).
    static qint64 directorySize(const QString &path);

    // Timestamp of the last commit for the git repository at `path`.
    // Returns an invalid QDateTime if it cannot be determined.
    static QDateTime lastCommitTime(const QString &path);

    // Most recent modification time found by recursively walking `path`.
    static QDateTime lastModifiedRecursive(const QString &path);

private:
    static ProjectInfo makeProjectInfo(const QString &path, const QString &group);
};
