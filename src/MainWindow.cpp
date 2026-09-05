#include "MainWindow.h"
#include "ProjectScanner.h"
#include "SettingsDialog.h"
#include "WorkspaceResolver.h"

#include <QAction>
#include <QFont>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr int kPathDataRole = Qt::UserRole + 1;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();
    refresh();
}

void MainWindow::buildUi()
{
    setWindowTitle(tr("kore - Workspace Panel"));
    resize(900, 600);

    auto *splitter = new QSplitter(Qt::Horizontal, this);

    m_tree = new QTreeWidget(splitter);
    m_tree->setColumnCount(4);
    m_tree->setHeaderLabels({tr("Project"), tr("Size"), tr("Last Update"), tr("Last Opened")});
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setAlternatingRowColors(true);
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showProjectContextMenu);
    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &MainWindow::openInIde);

    m_recentList = new QListWidget(splitter);
    m_recentList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_recentList, &QListWidget::customContextMenuRequested, this, &MainWindow::showRecentContextMenu);
    connect(m_recentList, &QListWidget::itemDoubleClicked, this, &MainWindow::openInIde);

    splitter->addWidget(m_tree);
    splitter->addWidget(m_recentList);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);

    QToolBar *toolBar = addToolBar(tr("Main"));
    QAction *refreshAction = toolBar->addAction(tr("Refresh"));
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refresh);
    QAction *settingsAction = toolBar->addAction(tr("Settings"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::openSettingsDialog);

    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::refresh()
{
    m_workspaceRoot = WorkspaceResolver::resolve(m_settings.workspaceOverride());
    m_projects = ProjectScanner::scan(m_workspaceRoot);

    m_projectsByPath.clear();
    for (const ProjectInfo &info : m_projects) {
        ProjectInfo copy = info;
        copy.lastOpenTime = m_settings.lastOpenTime(info.path);
        m_projectsByPath.insert(info.path, copy);
    }

    m_statusLabel->setText(tr("Workspace: %1 (%2 projects)").arg(m_workspaceRoot).arg(m_projects.size()));

    populateProjectTree();
    populateRecentList();
}

void MainWindow::populateProjectTree()
{
    m_tree->clear();

    QHash<QString, QTreeWidgetItem *> groupItems;

    // Ensure groups are created in a stable, alphabetical order.
    QStringList groupNames;
    for (const ProjectInfo &info : m_projects) {
        if (!info.group.isEmpty() && !groupNames.contains(info.group)) {
            groupNames.append(info.group);
        }
    }
    groupNames.sort(Qt::CaseInsensitive);
    for (const QString &group : groupNames) {
        auto *groupItem = new QTreeWidgetItem(m_tree, {group});
        QFont font = groupItem->font(0);
        font.setBold(true);
        groupItem->setFont(0, font);
        groupItems.insert(group, groupItem);
    }

    for (const ProjectInfo &info : m_projectsByPath) {
        QTreeWidgetItem *parentItem = info.group.isEmpty() ? m_tree->invisibleRootItem() : groupItems.value(info.group);
        auto *item = new QTreeWidgetItem(parentItem);
        item->setText(0, info.name);
        item->setText(1, formatSize(info.sizeBytes));
        item->setText(2, formatTimestamp(info.lastUpdateTime));
        item->setText(3, formatTimestamp(info.lastOpenTime));
        item->setData(0, kPathDataRole, info.path);
        item->setToolTip(0, info.path);
    }

    m_tree->expandAll();
}

void MainWindow::populateRecentList()
{
    m_recentList->clear();
    const QStringList recent = m_settings.recentProjects();
    for (const QString &path : recent) {
        const ProjectInfo info = m_projectsByPath.value(path);
        const QString label = info.name.isEmpty() ? path : info.name;
        auto *item = new QListWidgetItem(label, m_recentList);
        item->setData(kPathDataRole, path);
        item->setToolTip(path);
    }
}

QString MainWindow::selectedProjectPath() const
{
    if (m_recentList->hasFocus() && m_recentList->currentItem()) {
        return m_recentList->currentItem()->data(kPathDataRole).toString();
    }
    if (m_tree->currentItem()) {
        const QString path = m_tree->currentItem()->data(0, kPathDataRole).toString();
        if (!path.isEmpty()) {
            // Only leaf project items carry a path; group headers do not.
            return path;
        }
    }
    if (m_recentList->currentItem()) {
        return m_recentList->currentItem()->data(kPathDataRole).toString();
    }
    return QString();
}

void MainWindow::openProjectWithCommand(const QString &path, const QString &commandTemplate)
{
    if (path.isEmpty()) {
        return;
    }
    // The path is shell-quoted before substitution so that project
    // directory names containing shell metacharacters (spaces, quotes,
    // `;`, `$()`, backticks, etc.) cannot be interpreted by the shell.
    const QString command = QString(commandTemplate).arg(shellQuote(path));
#if defined(Q_OS_WIN)
    QProcess::startDetached(QStringLiteral("cmd"), {QStringLiteral("/C"), command});
#else
    QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), command});
#endif
    m_settings.recordOpened(path);
    populateRecentList();
    populateProjectTree();
}

void MainWindow::openInIde()
{
    openProjectWithCommand(selectedProjectPath(), m_settings.ideCommand());
}

void MainWindow::openInTerminal()
{
    openProjectWithCommand(selectedProjectPath(), m_settings.terminalCommand());
}

void MainWindow::openInOpenCode()
{
    openProjectWithCommand(selectedProjectPath(), m_settings.openCodeCommand());
}

void MainWindow::showProjectContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_tree->itemAt(pos);
    if (!item || item->data(0, kPathDataRole).toString().isEmpty()) {
        return;
    }
    m_tree->setCurrentItem(item);

    QMenu menu(this);
    menu.addAction(tr("Open in IDE"), this, &MainWindow::openInIde);
    menu.addAction(tr("Open in Terminal"), this, &MainWindow::openInTerminal);
    menu.addAction(tr("Open in OpenCode"), this, &MainWindow::openInOpenCode);
    menu.exec(m_tree->viewport()->mapToGlobal(pos));
}

void MainWindow::showRecentContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_recentList->itemAt(pos);
    if (!item) {
        return;
    }
    m_recentList->setCurrentItem(item);

    QMenu menu(this);
    menu.addAction(tr("Open in IDE"), this, &MainWindow::openInIde);
    menu.addAction(tr("Open in Terminal"), this, &MainWindow::openInTerminal);
    menu.addAction(tr("Open in OpenCode"), this, &MainWindow::openInOpenCode);
    menu.exec(m_recentList->viewport()->mapToGlobal(pos));
}

void MainWindow::openSettingsDialog()
{
    SettingsDialog dialog(&m_settings, this);
    if (dialog.exec() == QDialog::Accepted) {
        refresh();
    }
}

QString MainWindow::formatSize(qint64 bytes)
{
    constexpr qint64 kKiB = 1024;
    constexpr qint64 kMiB = kKiB * 1024;
    constexpr qint64 kGiB = kMiB * 1024;

    if (bytes >= kGiB) {
        return QStringLiteral("%1 GiB").arg(static_cast<double>(bytes) / kGiB, 0, 'f', 2);
    }
    if (bytes >= kMiB) {
        return QStringLiteral("%1 MiB").arg(static_cast<double>(bytes) / kMiB, 0, 'f', 2);
    }
    if (bytes >= kKiB) {
        return QStringLiteral("%1 KiB").arg(static_cast<double>(bytes) / kKiB, 0, 'f', 2);
    }
    return QStringLiteral("%1 B").arg(bytes);
}

QString MainWindow::formatTimestamp(const QDateTime &dt)
{
    if (!dt.isValid()) {
        return tr("never");
    }
    return dt.toString(Qt::ISODate);
}

QString MainWindow::shellQuote(const QString &path)
{
#if defined(Q_OS_WIN)
    // cmd.exe: wrap in double quotes, doubling any embedded double quotes.
    QString escaped = path;
    escaped.replace(QLatin1String("\""), QLatin1String("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
#else
    // POSIX sh: single-quote the value, escaping embedded single quotes by
    // closing the quote, emitting an escaped quote, and reopening it.
    QString escaped = path;
    escaped.replace(QLatin1String("'"), QLatin1String("'\\''"));
    return QLatin1Char('\'') + escaped + QLatin1Char('\'');
#endif
}
