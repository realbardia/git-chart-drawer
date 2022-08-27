#ifndef GITCOMMANDS_H
#define GITCOMMANDS_H

#include <QDateTime>
#include <QObject>
#include <functional>

class GitCommands : public QObject
{
    Q_OBJECT
public:
    struct Commit {
        QString id;
        QDateTime datetime;
        QString commiter;
        QString comment;
    };

    struct Stat {
        QString fileName;
        qint32 insertions = 0;
        qint32 deletions = 0;
    };

    GitCommands(QObject *parent = nullptr);
    GitCommands(const QString &path, QObject *parent = nullptr);
    virtual ~GitCommands();

    void listCommits(std::function<void(QList<Commit>)> callback);
    void commitDiff(const QString &commit, std::function<void(QString)> callback);
    void commitStat(const QString &commit, std::function<void(QList<Stat>)> callback);

    QString path() const;
    void setPath(const QString &newPath);

Q_SIGNALS:


private:
    QString mPath;
};

#endif // GITCOMMANDS_H
