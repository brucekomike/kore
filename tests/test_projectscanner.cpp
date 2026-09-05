#include "ProjectScanner.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

class TestProjectScanner : public QObject
{
    Q_OBJECT

private slots:
    void detectsUngroupedGitRepo();
    void detectsGroupedProjects();
    void computesDirectorySize();

private:
    static void writeFile(const QString &path, const QByteArray &content);
};

void TestProjectScanner::writeFile(const QString &path, const QByteArray &content)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(content);
    file.close();
}

void TestProjectScanner::detectsUngroupedGitRepo()
{
    QTemporaryDir workspace;
    QVERIFY(workspace.isValid());

    QDir root(workspace.path());
    QVERIFY(root.mkpath("repoA/.git"));
    writeFile(workspace.path() + "/repoA/README.md", "hello");

    const ProjectList projects = ProjectScanner::scan(workspace.path());
    QCOMPARE(projects.size(), 1);
    QCOMPARE(projects.first().name, QStringLiteral("repoA"));
    QVERIFY(projects.first().group.isEmpty());
    QVERIFY(projects.first().isGitRepo);
}

void TestProjectScanner::detectsGroupedProjects()
{
    QTemporaryDir workspace;
    QVERIFY(workspace.isValid());

    QDir root(workspace.path());
    QVERIFY(root.mkpath("work/serviceA/.git"));
    QVERIFY(root.mkpath("work/serviceB"));
    writeFile(workspace.path() + "/work/serviceA/main.cpp", "int main(){}");
    writeFile(workspace.path() + "/work/serviceB/main.cpp", "int main(){}");

    const ProjectList projects = ProjectScanner::scan(workspace.path());
    QCOMPARE(projects.size(), 2);

    bool foundA = false;
    bool foundB = false;
    for (const ProjectInfo &info : projects) {
        if (info.name == QStringLiteral("serviceA")) {
            foundA = true;
            QCOMPARE(info.group, QStringLiteral("work"));
            QVERIFY(info.isGitRepo);
        } else if (info.name == QStringLiteral("serviceB")) {
            foundB = true;
            QCOMPARE(info.group, QStringLiteral("work"));
            QVERIFY(!info.isGitRepo);
        }
    }
    QVERIFY(foundA);
    QVERIFY(foundB);
}

void TestProjectScanner::computesDirectorySize()
{
    QTemporaryDir workspace;
    QVERIFY(workspace.isValid());

    QDir root(workspace.path());
    QVERIFY(root.mkpath("repoA/.git"));
    const QByteArray content(1234, 'x');
    writeFile(workspace.path() + "/repoA/data.bin", content);

    const ProjectList projects = ProjectScanner::scan(workspace.path());
    QCOMPARE(projects.size(), 1);
    QVERIFY(projects.first().sizeBytes >= content.size());
}

QTEST_APPLESS_MAIN(TestProjectScanner)
#include "test_projectscanner.moc"
