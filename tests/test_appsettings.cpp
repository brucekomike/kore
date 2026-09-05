#include "AppSettings.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

class TestAppSettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void defaultsAreSensible();
    void persistsWorkspaceOverride();
    void recordsRecentProjectsMostRecentFirst();
    void tracksLastOpenTimePerProject();

private:
    QTemporaryDir m_settingsDir;
};

void TestAppSettings::initTestCase()
{
    QVERIFY(m_settingsDir.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDir.path());
}

void TestAppSettings::cleanupTestCase()
{
}

void TestAppSettings::init()
{
    // Start each test with a clean slate.
    AppSettings settings;
    settings.setWorkspaceOverride(QString());
    settings.setIdeCommand(QString());
    settings.setTerminalCommand(QString());
    settings.setOpenCodeCommand(QString());
}

void TestAppSettings::defaultsAreSensible()
{
    AppSettings settings;
    settings.setIdeCommand(QString());
    settings.setTerminalCommand(QString());
    settings.setOpenCodeCommand(QString());
    QVERIFY(settings.workspaceOverride().isEmpty());
}

void TestAppSettings::persistsWorkspaceOverride()
{
    AppSettings settings;
    settings.setWorkspaceOverride(QStringLiteral("/tmp/some-workspace"));
    QCOMPARE(settings.workspaceOverride(), QStringLiteral("/tmp/some-workspace"));
}

void TestAppSettings::recordsRecentProjectsMostRecentFirst()
{
    AppSettings settings;
    settings.recordOpened(QStringLiteral("/tmp/project-a"));
    settings.recordOpened(QStringLiteral("/tmp/project-b"));
    settings.recordOpened(QStringLiteral("/tmp/project-a")); // re-opened, should move to front

    const QStringList recent = settings.recentProjects();
    QVERIFY(recent.size() >= 2);
    QCOMPARE(recent.first(), QStringLiteral("/tmp/project-a"));
    QCOMPARE(recent.at(1), QStringLiteral("/tmp/project-b"));
}

void TestAppSettings::tracksLastOpenTimePerProject()
{
    AppSettings settings;
    QVERIFY(!settings.lastOpenTime(QStringLiteral("/tmp/never-opened")).isValid());

    settings.recordOpened(QStringLiteral("/tmp/project-c"));
    QVERIFY(settings.lastOpenTime(QStringLiteral("/tmp/project-c")).isValid());
}

QTEST_APPLESS_MAIN(TestAppSettings)
#include "test_appsettings.moc"
