#include "commitchartwidget.h"

#include <QtMath>
#include <QDir>
#include <QCryptographicHash>
#include <QImageWriter>
#include <QJsonDocument>

using namespace QtCharts;

CommitChartWidget::CommitChartWidget(QWidget *parent)
    : AbstractChartWidget(parent)
{
    mGit = new GitCommands(this);
}

CommitChartWidget::~CommitChartWidget()
{
}

void CommitChartWidget::load(const QString &path)
{
    mGit->setPath(path);
    mGit->listCommits([path, this](const QList<GitCommands::Commit> &list){
        Q_EMIT loading(true, 0, list.count());
        loadCommits(path, list);
    });

    Q_EMIT loading(true, 0, 0);
}

void CommitChartWidget::remove(const QString &path)
{
    mAnalizeds.remove(path);
    reload();
}

void CommitChartWidget::loadCommits(const QString &fileName, const QList<GitCommands::Commit> &list)
{
    if (list.isEmpty())
    {
        finished();
        Q_EMIT loading(false, 0, 0);
        return;
    }

    const auto c = list.first();
    mGit->commitStat(c.id, [c, this, list, fileName](const QList<GitCommands::Stat> &stats){
        AnalizedCommit a;
        a.commit = c;
        a.totalFiles = stats.count();

        for (const auto &s: stats)
        {
            a.deletions += s.deletions;
            a.insertions += s.insertions;
        }

        mAnalizeds[fileName] << a;

        Q_EMIT loading(true, mAnalizeds.count(), mAnalizeds.count() + list.count() - 1);
        loadCommits(fileName, list.mid(1));
    });
}

void CommitChartWidget::finished()
{
    reload();
}

const QDateTime &CommitChartWidget::maxDate() const
{
    return mMaxDate;
}

bool CommitChartWidget::saveTo(const QString &path, int w)
{
    qreal ratio = (qreal)w / width();

    QImage img(w, height()*ratio, QImage::Format_ARGB32);
    img.fill(QColor(0,0,0,0));
    img.setDevicePixelRatio(ratio);

    render(&img, QPoint(), QRegion(rect()), QWidget::DrawChildren);

    QFile::remove(path);
    QImageWriter writer(path);
    return writer.write(img);
}

bool CommitChartWidget::saveJson(const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::WriteOnly))
        return false;

    QVariantList list;

    QHashIterator<QString, QList<AnalizedCommit>> i(mAnalizeds);
    while (i.hasNext())
    {
        i.next();

        QVariantList commits;
        for (const auto &a: i.value())
        {
            QVariantMap c;
            c[QStringLiteral("commiter")] = a.commit.commiter;
            c[QStringLiteral("comment")] = a.commit.comment;
            c[QStringLiteral("datetime")] = a.commit.datetime;
            c[QStringLiteral("id")] = a.commit.id;
            c[QStringLiteral("deletions")] = a.deletions;
            c[QStringLiteral("insertions")] = a.insertions;
            c[QStringLiteral("total_files")] = a.totalFiles;

            commits << c;
        }

        QVariantMap m;
        m[QStringLiteral("git_repo")] = i.key();
        m[QStringLiteral("commits")] = commits;

        list << m;
    }

    f.write( QJsonDocument::fromVariant(list).toJson() );
    f.close();
    return true;
}

bool CommitChartWidget::saveCSV(const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::WriteOnly))
        return false;

    QString data;

    QHashIterator<QString, QList<AnalizedCommit>> i(mAnalizeds);
    while (i.hasNext())
    {
        i.next();
        if (!data.isEmpty())
            data += QStringLiteral("\n");

        data += QStringLiteral("%1,Commiter,Date/Time,Comment,Total Files,Insertions,Deletions\n").arg(i.key());

        for (const auto &a: i.value())
        {
            data += QStringLiteral("%1,%2,%3,%4,%5,%6,%7\n")
                    .arg(a.commit.id)
                    .arg(a.commit.commiter)
                    .arg(a.commit.datetime.toString("yyyy/MM/dd hh:mm:ss"))
                    .arg(a.commit.comment)
                    .arg(a.totalFiles)
                    .arg(a.insertions)
                    .arg(a.deletions);
        }
    }

    f.write(data.toUtf8());
    f.close();
    return true;
}

const QDateTime &CommitChartWidget::minDate() const
{
    return mMinDate;
}

CommitChartWidget::ViewType CommitChartWidget::viewType() const
{
    return mViewType;
}

void CommitChartWidget::setViewType(ViewType newViewType)
{
    mViewType = newViewType;
}

void CommitChartWidget::reload()
{
    QHash<QString, AbstractChartWidget::SeriesUnit> points;

    mMinDate = QDateTime::currentDateTime();
    mMaxDate = QDateTime(QDate(1,1,1), QTime(0,0,0));

    QHashIterator<QString, QList<AnalizedCommit>> i(mAnalizeds);
    while (i.hasNext())
    {
        i.next();
        QDir inf(i.key());
        const auto fileName = inf.dirName();

        for (const auto &a: i.value())
        {
            mMinDate = std::min(mMinDate, a.commit.datetime);
            mMaxDate = std::max(mMaxDate, a.commit.datetime);

            PointValue p;

            switch (static_cast<int>(mViewType))
            {
            case ViewType::ViewCommiters:
            {
                p.datetime = a.commit.datetime;

                switch (static_cast<int>(mDataType))
                {
                case DataType::Changes:
                    p.value = a.insertions + a.deletions;
                    break;

                case DataType::Files:
                    p.value = a.totalFiles;
                    break;
                }

                auto &s = points[fileName + QStringLiteral("\n") + a.commit.commiter];
                s.title = a.commit.commiter;
                s.category = fileName;
                s.points << p;
            }
                break;

            case ViewType::ViewOverall:
            {
                switch (static_cast<int>(mDataType))
                {
                case DataType::Changes:
                {
                    p.datetime = a.commit.datetime;
                    p.value = a.insertions;

                    auto &s0 = points[ fileName + QStringLiteral("\ninsertions") ];
                    s0.title = QStringLiteral("Insertions");
                    s0.category = fileName;
                    s0.points << p;

                    p.datetime = a.commit.datetime;
                    p.value = a.deletions;

                    auto &s1 = points[ fileName + QStringLiteral("\ndeletions") ];
                    s1.title = QStringLiteral("Deletions");
                    s1.category = fileName;
                    s1.points << p;

                    p.datetime = a.commit.datetime;
                    p.value = a.insertions + a.deletions;

                    auto &s2 = points[ fileName + QStringLiteral("\ntotal") ];
                    s2.title = QStringLiteral("Total");
                    s2.category = fileName;
                    s2.points << p;
                }
                    break;

                case DataType::Files:
                {
                    p.datetime = a.commit.datetime;
                    p.value = a.totalFiles;

                    auto &s = points[ fileName + QStringLiteral("\nfiles") ];
                    s.title = QStringLiteral("Files");
                    s.category = fileName;
                    s.points << p;
                }
                    break;
                }
            }
                break;
            }
        }
    }

    setStartDate(mMinDate);
    setEndDate(mMaxDate);
    setStackable(true);

    QList<AbstractChartWidget::SeriesUnit> units;
    for (auto p: points)
    {
        p.uniqueId = QCryptographicHash::hash((p.category + "\n" + p.title).toUtf8(), QCryptographicHash::Md5).toHex();
        units << p;
    }
    setPoints(units);

    AbstractChartWidget::reload();
}

CommitChartWidget::DataType CommitChartWidget::dataType() const
{
    return mDataType;
}

void CommitChartWidget::setDataType(DataType newDataType)
{
    mDataType = newDataType;
}
