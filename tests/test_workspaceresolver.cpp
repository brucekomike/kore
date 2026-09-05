#include "WorkspaceResolver.h"

#include <QDir>
#include <QTemporaryDir>
#include <QTest>

class TestWorkspaceResolver : public QObject
{
    Q_OBJECT

private slots:
    void overrideTakesPrecedence();
    void ignoresNonExistentOverride();
    void fallsBackToHomeWhenNoCandidateExists();
};

void TestWorkspaceResolver::overrideTakesPrecedence()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString resolved = WorkspaceResolver::resolve(tempDir.path());
    QCOMPARE(resolved, QDir(tempDir.path()).absolutePath());
}

void TestWorkspaceResolver::ignoresNonExistentOverride()
{
    const QString resolved = WorkspaceResolver::resolve(QStringLiteral("/this/path/does/not/exist/hopefully"));
    // Should fall back to one of the auto-resolved candidates rather than
    // returning the bogus path.
    QVERIFY(resolved != QStringLiteral("/this/path/does/not/exist/hopefully"));
    QVERIFY(QDir(resolved).exists());
}

void TestWorkspaceResolver::fallsBackToHomeWhenNoCandidateExists()
{
    // When no override and (presumably) no ~/Workspace or ~/Projects exist in
    // this sandboxed test environment, resolve() must still return an
    // existing directory (home, at worst).
    const QString resolved = WorkspaceResolver::resolve();
    QVERIFY(QDir(resolved).exists());
}

QTEST_APPLESS_MAIN(TestWorkspaceResolver)
#include "test_workspaceresolver.moc"
