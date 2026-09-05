#pragma once

#include <QDialog>

class AppSettings;

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

// Simple dialog allowing the user to configure the workspace folder
// override and the commands used to open a project in an IDE, CLI, or agent
// IDE.
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(AppSettings *settings, QWidget *parent = nullptr);

private slots:
    void browseWorkspace();
    void detectIde();
    void detectTerminal();
    void detectAgentIde();
    void save();

private:
    AppSettings *m_settings;
    QLineEdit *m_workspaceEdit = nullptr;
    QLineEdit *m_ideEdit = nullptr;
    QLineEdit *m_terminalEdit = nullptr;
    QLineEdit *m_openCodeEdit = nullptr;
};
