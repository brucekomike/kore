#pragma once

#include "AppSettings.h"
#include "ProjectInfo.h"

#include <QHash>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QLabel;
class QFrame;
class QScrollArea;
class QVBoxLayout;
QT_END_NAMESPACE

// Main panel window: shows the workspace projects grouped by subfolder, a
// list of recently opened projects, and actions to open a project in the
// user's configured IDE, CLI, or agent IDE.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void refresh();
    void openSettingsDialog();
    void navigateToGroup(QListWidgetItem *item);
    void showRecentContextMenu(const QPoint &pos);
    void openInIde();
    void openInTerminal();
    void openInOpenCode();

private:
    void buildUi();
    void populateProjectTree();
    void populateRecentList();
    QFrame *makeProjectCard(const ProjectInfo &info);
    QString selectedProjectPath() const;
    void openProjectWithCommand(const QString &path, const QString &commandTemplate);
    static QString formatSize(qint64 bytes);
    static QString formatTimestamp(const QDateTime &dt);
    static QString shellQuote(const QString &path);

    AppSettings m_settings;
    QString m_workspaceRoot;
    ProjectList m_projects;
    QHash<QString, ProjectInfo> m_projectsByPath;

    QWidget *m_projectCards = nullptr;
    QVBoxLayout *m_projectLayout = nullptr;
    QScrollArea *m_projectScroll = nullptr;
    QListWidget *m_groupIndex = nullptr;
    QListWidget *m_recentList = nullptr;
    QHash<QString, QWidget *> m_groupCards;
    QLabel *m_statusLabel = nullptr;
    QString m_selectedProjectPath;
};
