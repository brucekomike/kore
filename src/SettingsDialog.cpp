#include "SettingsDialog.h"
#include "AppSettings.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(AppSettings *settings, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
{
    setWindowTitle(tr("kore Settings"));

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout();

    auto *workspaceRow = new QHBoxLayout();
    m_workspaceEdit = new QLineEdit(m_settings->workspaceOverride(), this);
    m_workspaceEdit->setPlaceholderText(tr("Auto-detect (~/Workspace or ~/Projects)"));
    auto *browseButton = new QPushButton(tr("Browse..."), this);
    connect(browseButton, &QPushButton::clicked, this, &SettingsDialog::browseWorkspace);
    workspaceRow->addWidget(m_workspaceEdit);
    workspaceRow->addWidget(browseButton);
    form->addRow(tr("Workspace folder:"), workspaceRow);

    m_ideEdit = new QLineEdit(m_settings->ideCommand(), this);
    m_ideEdit->setPlaceholderText(tr("e.g. code %1"));
    auto *ideRow = new QHBoxLayout();
    ideRow->addWidget(m_ideEdit);
    auto *detectIdeButton = new QPushButton(tr("Auto-detect"), this);
    connect(detectIdeButton, &QPushButton::clicked, this, &SettingsDialog::detectIde);
    ideRow->addWidget(detectIdeButton);
    form->addRow(tr("IDE command template:"), ideRow);

    m_terminalEdit = new QLineEdit(m_settings->terminalCommand(), this);
    m_terminalEdit->setPlaceholderText(tr("e.g. open -a Terminal %1"));
    auto *terminalRow = new QHBoxLayout();
    terminalRow->addWidget(m_terminalEdit);
    auto *detectTerminalButton = new QPushButton(tr("Auto-detect"), this);
    connect(detectTerminalButton, &QPushButton::clicked, this, &SettingsDialog::detectTerminal);
    terminalRow->addWidget(detectTerminalButton);
    form->addRow(tr("CLI command template:"), terminalRow);

    m_openCodeEdit = new QLineEdit(m_settings->openCodeCommand(), this);
    m_openCodeEdit->setPlaceholderText(tr("e.g. opencode %1"));
    auto *agentIdeRow = new QHBoxLayout();
    agentIdeRow->addWidget(m_openCodeEdit);
    auto *detectAgentIdeButton = new QPushButton(tr("Auto-detect"), this);
    connect(detectAgentIdeButton, &QPushButton::clicked, this, &SettingsDialog::detectAgentIde);
    agentIdeRow->addWidget(detectAgentIdeButton);
    form->addRow(tr("Agent IDE command template:"), agentIdeRow);

    layout->addLayout(form);
    layout->addWidget(new QLabel(tr("Use %1 as a placeholder for the project path; it is automatically shell-quoted, so do not add your own quotes around it.").arg(QStringLiteral("%1")), this));

    auto *clearHistoryButton = new QPushButton(tr("Clear Recent History"), this);
    connect(clearHistoryButton, &QPushButton::clicked, this, &SettingsDialog::clearRecentHistory);
    layout->addWidget(clearHistoryButton);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::save);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void SettingsDialog::browseWorkspace()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Select Workspace Folder"), m_workspaceEdit->text());
    if (!dir.isEmpty()) {
        m_workspaceEdit->setText(dir);
    }
}

void SettingsDialog::detectIde()
{
    bool accepted = false;
    const QString choice = QInputDialog::getItem(this, tr("Choose IDE"),
                                                  tr("Detected IDE command template:"),
                                                  AppSettings::detectedIdeCommands(), 0, false, &accepted);
    if (accepted && !choice.isEmpty()) {
        m_ideEdit->setText(choice);
    }
}

void SettingsDialog::detectTerminal()
{
    bool accepted = false;
    const QString choice = QInputDialog::getItem(this, tr("Choose CLI"),
                                                  tr("Detected CLI command template:"),
                                                  AppSettings::detectedTerminalCommands(), 0, false, &accepted);
    if (accepted && !choice.isEmpty()) {
        m_terminalEdit->setText(choice);
    }
}

void SettingsDialog::detectAgentIde()
{
    bool accepted = false;
    const QString choice = QInputDialog::getItem(this, tr("Choose Agent IDE"),
                                                  tr("Detected Agent IDE command template:"),
                                                  AppSettings::detectedAgentIdeCommands(), 0, false, &accepted);
    if (accepted && !choice.isEmpty()) {
        m_openCodeEdit->setText(choice);
    }
}

void SettingsDialog::clearRecentHistory()
{
    if (QMessageBox::question(this, tr("Clear Recent History"),
                              tr("Remove all recently opened projects?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        m_settings->clearRecentProjects();
    }
}

void SettingsDialog::save()
{
    m_settings->setWorkspaceOverride(m_workspaceEdit->text());
    m_settings->setIdeCommand(m_ideEdit->text());
    m_settings->setTerminalCommand(m_terminalEdit->text());
    m_settings->setOpenCodeCommand(m_openCodeEdit->text());
    accept();
}
