#include "MainWindow.h"
#include "ProjectScanner.h"
#include "SettingsDialog.h"
#include "WorkspaceResolver.h"

#include <QAction>
#include <QFrame>
#include <QFont>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QDir>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QScrollArea>
#include <QScrollBar>
#include <QSizePolicy>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QPushButton>
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

    auto *projectPanel = new QWidget(splitter);
    auto *projectPanelLayout = new QVBoxLayout(projectPanel);
    auto *projectsTitle = new QLabel(tr("Projects"), projectPanel);
    QFont projectsTitleFont = projectsTitle->font();
    projectsTitleFont.setBold(true);
    projectsTitle->setFont(projectsTitleFont);
    projectPanelLayout->addWidget(projectsTitle);

    m_projectScroll = new QScrollArea(projectPanel);
    m_projectScroll->setWidgetResizable(true);
    m_projectScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_projectCards = new QWidget(m_projectScroll);
    m_projectLayout = new QVBoxLayout(m_projectCards);
    m_projectLayout->setContentsMargins(8, 8, 8, 8);
    m_projectLayout->setSpacing(10);
    m_projectLayout->addStretch();
    m_projectScroll->setWidget(m_projectCards);
    projectPanelLayout->addWidget(m_projectScroll, 1);

    auto *rightPanel = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    auto *contentsTitle = new QLabel(tr("Contents"), rightPanel);
    QFont contentsTitleFont = contentsTitle->font();
    contentsTitleFont.setBold(true);
    contentsTitle->setFont(contentsTitleFont);
    rightLayout->addWidget(contentsTitle);

    m_groupIndex = new QListWidget(rightPanel);
    connect(m_groupIndex, &QListWidget::itemClicked, this, &MainWindow::navigateToGroup);
    rightLayout->addWidget(m_groupIndex, 1);

    auto *recentTitle = new QLabel(tr("Recently Opened"), rightPanel);
    QFont recentTitleFont = recentTitle->font();
    recentTitleFont.setBold(true);
    recentTitle->setFont(recentTitleFont);
    rightLayout->addWidget(recentTitle);

    m_recentList = new QListWidget(rightPanel);
    m_recentList->setMaximumHeight(5 * m_recentList->fontMetrics().lineSpacing()
                                   + 2 * m_recentList->frameWidth() + 16);
    m_recentList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_recentList, &QListWidget::customContextMenuRequested, this, &MainWindow::showRecentContextMenu);
    connect(m_recentList, &QListWidget::itemDoubleClicked, this, &MainWindow::openInIde);

    rightLayout->addWidget(m_recentList, 1);

    splitter->addWidget(projectPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({700, 260});

    setCentralWidget(splitter);

    QToolBar *toolBar = addToolBar(tr("Main"));
    QAction *refreshAction = toolBar->addAction(tr("Refresh"));
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refresh);
    QAction *settingsAction = toolBar->addAction(tr("Settings"));
#if defined(Q_OS_MAC)
    settingsAction->setShortcut(QKeySequence(Qt::META | Qt::Key_Comma));
#else
    settingsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Comma));
#endif
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

QFrame *MainWindow::makeProjectCard(const ProjectInfo &info)
{
    auto *card = new QFrame(m_projectCards);
    card->setFrameShape(QFrame::StyledPanel);
    card->setFrameShadow(QFrame::Raised);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(12, 10, 8, 10);

    auto *details = new QVBoxLayout;
    const QString relativePath = QDir(m_workspaceRoot).relativeFilePath(info.path);
    auto *titleRow = new QHBoxLayout;
    auto *pathLabel = new QLabel(relativePath, card);
    pathLabel->setWordWrap(false);
    pathLabel->setMinimumWidth(0);
    pathLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    pathLabel->setText(QFontMetrics(pathLabel->font()).elidedText(relativePath, Qt::ElideRight, 220));
    pathLabel->setToolTip(info.path);

    auto *name = new QLabel(info.name, card);
    QFont nameFont = name->font();
    nameFont.setBold(true);
    name->setFont(nameFont);
    name->setMinimumWidth(40);
    name->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    name->setText(QFontMetrics(name->font()).elidedText(info.name, Qt::ElideRight, 220));
    name->setToolTip(info.name);
    titleRow->addWidget(name, 0);
    titleRow->addWidget(pathLabel, 1);
    details->addLayout(titleRow);

    const QString metadataText = tr("Size: %1    Modified: %2    Opened: %3")
            .arg(formatSize(info.sizeBytes))
            .arg(formatTimestamp(info.lastUpdateTime))
            .arg(formatTimestamp(info.lastOpenTime));
    auto *metadata = new QLabel(metadataText, card);
    metadata->setWordWrap(false);
    metadata->setMinimumWidth(0);
    metadata->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    metadata->setText(QFontMetrics(metadata->font()).elidedText(metadataText, Qt::ElideRight, 420));
    metadata->setToolTip(metadataText);
    details->addWidget(metadata);
    layout->addLayout(details, 1);

    auto *openActions = new QHBoxLayout;
    openActions->setSpacing(4);
    auto addOpenButton = [this, &openActions, &info](const QString &label, auto slot) {
        auto *button = new QPushButton(label, m_projectCards);
        button->setFixedHeight(26);
        button->setFixedWidth(label == tr("Agent IDE") ? 84 : 52);
        connect(button, &QPushButton::clicked, this, [this, path = info.path, slot] {
            m_selectedProjectPath = path;
            (this->*slot)();
        });
        openActions->addWidget(button);
    };
    addOpenButton(tr("IDE"), &MainWindow::openInIde);
    addOpenButton(tr("CLI"), &MainWindow::openInTerminal);
    addOpenButton(tr("Agent IDE"), &MainWindow::openInOpenCode);
    layout->addLayout(openActions, 0);
    return card;
}

void MainWindow::populateProjectTree()
{
    while (m_projectLayout->count() > 1) {
        delete m_projectLayout->takeAt(0)->widget();
    }
    m_groupCards.clear();
    m_groupIndex->clear();

    // Ensure groups are created in a stable, alphabetical order.
    QStringList groupNames;
    for (const ProjectInfo &info : m_projects) {
        if (!info.group.isEmpty() && !groupNames.contains(info.group)) {
            groupNames.append(info.group);
        }
    }
    groupNames.sort(Qt::CaseInsensitive);

    for (const QString &group : groupNames) {
        auto *groupBox = new QGroupBox(group, m_projectCards);
        auto *groupLayout = new QVBoxLayout(groupBox);
        groupLayout->setSpacing(8);
        for (const ProjectInfo &info : m_projectsByPath) {
            if (info.group == group) {
                groupLayout->addWidget(makeProjectCard(info));
            }
        }
        m_projectLayout->insertWidget(m_projectLayout->count() - 1, groupBox);
        m_groupCards.insert(group, groupBox);
        auto *indexItem = new QListWidgetItem(group, m_groupIndex);
        indexItem->setToolTip(tr("Jump to %1").arg(group));
    }

    for (const ProjectInfo &info : m_projectsByPath) {
        if (info.group.isEmpty()) {
            m_projectLayout->insertWidget(m_projectLayout->count() - 1, makeProjectCard(info));
        }
    }
}

void MainWindow::navigateToGroup(QListWidgetItem *item)
{
    if (!item || !m_projectScroll) {
        return;
    }
    if (QWidget *groupCard = m_groupCards.value(item->text())) {
        m_projectScroll->verticalScrollBar()->setValue(qMax(0, groupCard->geometry().top() - 8));
    }
}

void MainWindow::populateRecentList()
{
    m_recentList->clear();
    const QStringList recent = m_settings.recentProjects(5);
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
    if (!m_selectedProjectPath.isEmpty()) {
        return m_selectedProjectPath;
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

void MainWindow::showRecentContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_recentList->itemAt(pos);
    if (!item) {
        return;
    }
    m_recentList->setCurrentItem(item);

    QMenu menu(this);
    menu.addAction(tr("Open in IDE"), this, &MainWindow::openInIde);
    menu.addAction(tr("Open in CLI"), this, &MainWindow::openInTerminal);
    menu.addAction(tr("Open in Agent IDE"), this, &MainWindow::openInOpenCode);
    menu.addSeparator();
    menu.addAction(tr("Remove from Recent"), this, &MainWindow::removeRecentProject);
    menu.exec(m_recentList->viewport()->mapToGlobal(pos));
}

void MainWindow::removeRecentProject()
{
    if (!m_recentList->currentItem()) {
        return;
    }
    m_settings.removeRecentProject(m_recentList->currentItem()->data(kPathDataRole).toString());
    populateRecentList();
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
    return dt.toString(QStringLiteral("yyyy-MM-dd HH:mm"));
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
