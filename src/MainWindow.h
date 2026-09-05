#pragma once

#include "AppSettings.h"
#include "ProjectInfo.h"

#include <QHash>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QTreeWidget;
class QTreeWidgetItem;
class QListWidget;
class QListWidgetItem;
class QLabel;
QT_END_NAMESPACE

// Main panel window: shows the workspace projects grouped by subfolder, a
// list of recently opened projects, and actions to open a project in the
// user's configured IDE, terminal, or OpenCode desktop.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void refresh();
    void openSettingsDialog();
    void showProjectContextMenu(const QPoint &pos);
    void showRecentContextMenu(const QPoint &pos);
    void openInIde();
    void openInTerminal();
    void openInOpenCode();

private:
    void buildUi();
    void populateProjectTree();
    void populateRecentList();
    QString selectedProjectPath() const;
    void openProjectWithCommand(const QString &path, const QString &commandTemplate);
    static QString formatSize(qint64 bytes);
    static QString formatTimestamp(const QDateTime &dt);

    AppSettings m_settings;
    QString m_workspaceRoot;
    ProjectList m_projects;
    QHash<QString, ProjectInfo> m_projectsByPath;

    QTreeWidget *m_tree = nullptr;
    QListWidget *m_recentList = nullptr;
    QLabel *m_statusLabel = nullptr;
};
