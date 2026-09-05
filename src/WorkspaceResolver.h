#pragma once

#include <QString>

// Resolves the workspace root folder that kore should scan for projects.
//
// Resolution order:
//   1. An explicit override (e.g. coming from user settings), if non-empty
//      and pointing to an existing directory.
//   2. ~/Workspace, if it exists.
//   3. ~/Projects, if it exists.
//   4. The user's home directory as a last resort fallback so the
//      application always has something sensible to display.
class WorkspaceResolver
{
public:
    // Returns the resolved, absolute workspace root path.
    // `override` takes precedence when it is a non-empty, existing directory.
    static QString resolve(const QString &override = QString());

    // Candidate directory names (relative to home) considered during
    // auto-resolution, in priority order.
    static QStringList candidateNames();
};
