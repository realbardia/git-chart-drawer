#include "gitcommands.h"

#include <QProcess>
#include <QRegExp>
#include <QDebug>

GitCommands::GitCommands(QObject *parent)
    : QObject(parent)
{

}

GitCommands::GitCommands(const QString &path, QObject *parent)
    : GitCommands(parent)
{
    setPath(path);
}

GitCommands::~GitCommands()
{

}

void GitCommands::listCommits(std::function<void (QList<Commit>)> callback)
{
    auto p = new QProcess(this);
    p->setWorkingDirectory(mPath);
    p->setArguments({QStringLiteral("log"), QStringLiteral("--date=format:%Y-%m-%d %H:%M:%S")});
    p->setProgram(QStringLiteral("git"));

    connect(p, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [p, callback](int, QProcess::ExitStatus){
        const auto data = QString::fromUtf8(p->readAll());

        QRegExp rx("commit\\s+(.+)\\s+Author\\:\\s+(.+)\\s+Date\\:\\s+(.+)\\n\\n\\s+(.+)\\n\\n");
        rx.setMinimal(true);

        int pos = 0;
        QList<Commit> list;

        while ((pos = rx.indexIn(data, pos)) != -1)
        {
            Commit c;
            c.id = rx.cap(1).remove(QRegExp("Merge\\:.+")).trimmed();
            c.commiter = rx.cap(2).trimmed();
            c.datetime = QDateTime::fromString(rx.cap(3).trimmed(), QStringLiteral("yyyy-MM-dd hh:mm:ss"));
            c.comment = rx.cap(4).trimmed();

            list << c;
            pos += rx.matchedLength();
        }

        p->deleteLater();
        callback(list);
    });

    p->start();
}

void GitCommands::commitDiff(const QString &commit, std::function<void (QString)> callback)
{
    auto p = new QProcess(this);
    p->setWorkingDirectory(mPath);
    p->setArguments({QStringLiteral("diff"), (commit + QStringLiteral("^!"))});
    p->setProgram(QStringLiteral("git"));

    connect(p, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [p, callback](int, QProcess::ExitStatus){
        callback( QString::fromUtf8(p->readAll()) );
        p->deleteLater();
    });

    p->start();
}

void GitCommands::commitStat(const QString &commit, std::function<void (QList<Stat>)> callback)
{
    auto p = new QProcess(this);
    p->setWorkingDirectory(mPath);
    p->setArguments({QStringLiteral("diff"), (commit + QStringLiteral("^!")), QStringLiteral("--numstat")});
    p->setProgram(QStringLiteral("git"));

    connect(p, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [p, callback](int, QProcess::ExitStatus){
        const auto data = QString::fromUtf8(p->readAll());

        QRegExp rx("(\\d+)\\s+(\\d+)\\s+(.+)\\n");
        rx.setMinimal(true);

        int pos = 0;
        QList<Stat> list;

        while ((pos = rx.indexIn(data, pos)) != -1)
        {
            Stat c;
            c.insertions = rx.cap(1).trimmed().toInt();
            c.deletions = rx.cap(2).trimmed().toInt();
            c.fileName = rx.cap(3).trimmed();

            list << c;
            pos += rx.matchedLength();
        }

        p->deleteLater();
        callback(list);
    });

    p->start();
}

QString GitCommands::path() const
{
    return mPath;
}

void GitCommands::setPath(const QString &newPath)
{
    mPath = newPath;
}
