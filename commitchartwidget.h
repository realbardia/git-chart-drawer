#ifndef COMMITCHARTWIDGET_H
#define COMMITCHARTWIDGET_H

#include <QDateTime>
#include <QWidget>
#include <QChartView>
#include <QVBoxLayout>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QSplineSeries>

#include "gitcommands.h"
#include "abstractchartwidget.h"

class CommitChartWidget : public AbstractChartWidget
{
    Q_OBJECT
public:
    struct AnalizedCommit {
        GitCommands::Commit commit;

        qint32 insertions = 0;
        qint32 deletions = 0;
        qint32 totalFiles = 0;
    };

    enum ViewType {
        ViewOverall = 0,
        ViewCommiters = 1,
    };

    enum DataType {
        Changes = 0,
        Files = 1,
    };

    CommitChartWidget(QWidget *parent = nullptr);
    virtual ~CommitChartWidget();

    void load(const QString &path);
    void remove(const QString &path);

    DataType dataType() const;
    void setDataType(DataType newDataType);

    ViewType viewType() const;
    void setViewType(ViewType newViewType);

    virtual void reload() Q_DECL_OVERRIDE;

    const QDateTime &minDate() const;
    const QDateTime &maxDate() const;

public Q_SLOTS:
    bool saveTo(const QString &path, int w = 2500);
    bool saveJson(const QString &path);
    bool saveCSV(const QString &path);

Q_SIGNALS:
    void loading(bool state, qint32 done, qint32 total);

protected:
    void loadCommits(const QString &fileName, const QList<GitCommands::Commit> &list);
    void finished();

private:
    GitCommands *mGit = Q_NULLPTR;
    QHash<QString, QList<AnalizedCommit>> mAnalizeds;

    DataType mDataType = Changes;
    ViewType mViewType = ViewOverall;

    QDateTime mMinDate;
    QDateTime mMaxDate;
};

#endif // COMMITCHARTWIDGET_H
