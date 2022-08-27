#ifndef ABSTRACTCHARTWIDGET_H
#define ABSTRACTCHARTWIDGET_H

#include <QWidget>
#include <QChartView>
#include <QVBoxLayout>
#include <QDateTime>
#include <QSplineSeries>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>

class AbstractChartWidget : public QWidget
{
    Q_OBJECT
public:
    struct PointValue {
        qreal value = 0;
        QDateTime datetime;
    };

    enum Duration {
        Day = 0,
        Week = 1,
        Month = 2,
        Year = 3
    };

    struct SeriesUnit {
        QString uniqueId;
        QString category;
        QString title;
        QColor color;
        QList<AbstractChartWidget::PointValue> points;
    };

    AbstractChartWidget(QWidget *parent = nullptr);
    virtual ~AbstractChartWidget();

    QDateTime startDate() const;
    void setStartDate(const QDateTime &newStartDate);

    QDateTime endDate() const;
    void setEndDate(const QDateTime &newEndDate);

    Duration duration() const;
    void setDuration(Duration newDuration);

    bool stackable() const;
    void setStackable(bool newStackable);

    bool splineMode() const;
    void setSplineMode(bool newSplineMode);

public Q_SLOTS:
    void setPoints(const QList<SeriesUnit> &points);
    virtual void reload();

Q_SIGNALS:

private:
    QtCharts::QChartView *mChart;

    struct SeriesType {
        QtCharts::QLineSeries *series = Q_NULLPTR;
        qreal maxValue = 0;
        QDateTime minDate;
        QDateTime maxDate;
        SeriesUnit unit;
    };

    QHash<QString, SeriesType> mSeriesHash;
    QHash<QString, QWidget*> mLegends;

    QVBoxLayout *mLayout;
    QGridLayout *mLegendsLayout;
    Duration mDuration = Month;

    QList<SeriesUnit> mPoints;

    QDateTime mStartDate;
    QDateTime mEndDate;

    QtCharts::QDateTimeAxis *mAxisX = Q_NULLPTR;
    QtCharts::QValueAxis *mAxisY = Q_NULLPTR;

    bool mStackable = false;
    bool mSplineMode = false;

protected:
    SeriesType addSeries(const SeriesUnit &unit);
    void clearSeries();
    void removeSeries(const QString &category);
};

#endif // ABSTRACTCHARTWIDGET_H
