#include "SettingsDialog.h"
#include "AppSettings.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
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
    form->addRow(tr("Open in IDE command:"), m_ideEdit);

    m_terminalEdit = new QLineEdit(m_settings->terminalCommand(), this);
    form->addRow(tr("Open in Terminal command:"), m_terminalEdit);

    m_openCodeEdit = new QLineEdit(m_settings->openCodeCommand(), this);
    form->addRow(tr("Open in OpenCode command:"), m_openCodeEdit);

    layout->addLayout(form);
    layout->addWidget(new QLabel(tr("Use %1 as a placeholder for the project path in commands.").arg(QStringLiteral("%1")), this));

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

void SettingsDialog::save()
{
    m_settings->setWorkspaceOverride(m_workspaceEdit->text());
    m_settings->setIdeCommand(m_ideEdit->text());
    m_settings->setTerminalCommand(m_terminalEdit->text());
    m_settings->setOpenCodeCommand(m_openCodeEdit->text());
    accept();
}
