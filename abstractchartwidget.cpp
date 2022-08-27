#include "abstractchartwidget.h"

#include <QtMath>
#include <QLabel>
#include <QDebug>

using namespace QtCharts;

class ChartLegendItem: public QWidget
{
public:
    struct Legend {
        QColor color;
        QString title;
    };

    ChartLegendItem(const QString &title, QWidget *parent = Q_NULLPTR)
        : QWidget(parent)
    {
        mLegendsLayout = new QHBoxLayout;
        mLegendsLayout->setSpacing(4);
        mLegendsLayout->addStretch();

        auto f = font();
        f.setBold(true);
        f.setPointSize(f.pointSize() + 1);

        auto l = new QLabel(title);
        l->setFont(f);

        auto layout = new QVBoxLayout(this);
        layout->addWidget(l);
        layout->addLayout(mLegendsLayout);
    }

    void addLegend(const Legend &l)
    {
        auto plt = palette();
        plt.setBrush(QPalette::Base, l.color);

        auto square = new QWidget;
        square->setFixedSize(12, 12);
        square->setAutoFillBackground(true);
        square->setBackgroundRole(QPalette::Base);
        square->setPalette(plt);

        mLegendsLayout->insertWidget(mLegendsLayout->count()-1, square);
        mLegendsLayout->insertWidget(mLegendsLayout->count()-1, new QLabel(l.title));
        mLegendsLayout->insertSpacing(mLegendsLayout->count()-1, 10);
    }

private:
    QHBoxLayout *mLegendsLayout;
};

AbstractChartWidget::AbstractChartWidget(QWidget *parent)
    : QWidget(parent)
{
    mChart = new QChartView;

    auto chart = mChart->chart();
    chart->legend()->hide();
    chart->setBackgroundBrush(palette().base());

    mLegendsLayout = new QGridLayout;

    mLayout = new QVBoxLayout(this);
    mLayout->addLayout(mLegendsLayout);
    mLayout->addWidget(mChart);

    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
}

AbstractChartWidget::~AbstractChartWidget()
{

}

void AbstractChartWidget::setPoints(const QList<SeriesUnit> &activities)
{
    mPoints = activities;
}

void AbstractChartWidget::reload()
{
    clearSeries();

    qreal maxValue = 0;
    auto minDate = QDateTime::currentDateTime();
    auto maxDate = QDateTime(QDate(1,1,1), QTime(0,0,0));

    for (const auto &p: mPoints)
    {
        auto s = addSeries(p);
        maxValue = std::max(maxValue, s.maxValue);
        minDate = std::min(minDate, s.minDate);
        maxDate = std::max(maxDate, s.maxDate);
    }

    auto chart = mChart->chart();

    mAxisY = new QValueAxis(this);
    mAxisY->setMin(0);

    int maxValueLogPow = pow(10, floor(log10(maxValue)));

    mAxisY->setMax( (1 + floor(maxValue/maxValueLogPow)) * maxValueLogPow );

    mAxisX = new QDateTimeAxis(this);
    mAxisX->setFormat("yyyy MMM dd");
    mAxisX->setMin(minDate);
    mAxisX->setMax(maxDate);
    mAxisX->setTickCount( std::min<int>(6, minDate.daysTo(maxDate) / 30));

    for (auto s: mSeriesHash.values())
    {
        chart->addSeries(s.series);
        chart->setAxisX(mAxisX, s.series);
        chart->setAxisY(mAxisY, s.series);

        auto legend = static_cast<ChartLegendItem*>(mLegends.value(s.unit.category));
        if (!legend)
        {
            auto idx = mLegends.count();
            legend = new ChartLegendItem(s.unit.category);
            mLegendsLayout->addWidget(legend, idx/3, idx%3);
            mLegends[s.unit.category] = legend;
        }

        ChartLegendItem::Legend lgn;
        lgn.title = s.unit.title;
        lgn.color = s.series->color();

        legend->addLegend(lgn);
    }
}

bool AbstractChartWidget::splineMode() const
{
    return mSplineMode;
}

void AbstractChartWidget::setSplineMode(bool newSplineMode)
{
    mSplineMode = newSplineMode;
}

bool AbstractChartWidget::stackable() const
{
    return mStackable;
}

void AbstractChartWidget::setStackable(bool newStackable)
{
    mStackable = newStackable;
}

AbstractChartWidget::SeriesType AbstractChartWidget::addSeries(const SeriesUnit &unit)
{
    removeSeries(unit.uniqueId);

    std::map<QDateTime, qreal> durationMap;
    const auto currentDate = QDate::currentDate();
    const auto add_days = currentDate.daysInYear() - currentDate.dayOfYear();
    for (const auto &a: unit.points)
    {
        if (a.datetime < mStartDate)
            continue;
        if (a.datetime > mEndDate)
            continue;

        const auto date = a.datetime.date();
        switch (static_cast<int>(mDuration))
        {
        case Day:
            durationMap[a.datetime] += a.value;
            break;
        case Week:
        {
            auto dt = QDate::fromJulianDay(std::floor(date.toJulianDay() / 7) * 7 + 7);
            durationMap[QDateTime(dt, QTime(0,0,0))] += a.value;
        }
            break;
        case Month:
            durationMap[QDateTime(QDate(date.year(), date.month(), 1).addMonths(1).addDays(-1), QTime(0,0,0))] += a.value;
            break;
        case Year:
            durationMap[QDateTime(QDate(date.addDays(add_days).year()+1, 1, 1).addDays(-1), QTime(0,0,0))] += a.value;
            break;
        }
    }

    AbstractChartWidget::SeriesType s;
    s.unit = unit;
    s.maxValue = 0;
    s.minDate = QDateTime::currentDateTime();
    s.maxDate = QDateTime(QDate(1,1,1), QTime(0,0,0));

    s.series = (mSplineMode? new QSplineSeries(this) : new QLineSeries(this));
    s.series->setName(unit.uniqueId);
    if (unit.color.isValid())
        s.series->setColor(unit.color);

    qreal stack = 0;
    for (const auto &[k, v]: durationMap)
    {
        if (!mStackable)
            stack = v;
        else
            stack += v;

        s.minDate = std::min(s.minDate, k);
        s.maxDate = std::max(s.maxDate, k);
        s.maxValue = std::max(s.maxValue, stack);
        s.series->append(k.toMSecsSinceEpoch(), stack);
    }

    mSeriesHash[unit.uniqueId] = s;
    return s;
}

void AbstractChartWidget::clearSeries()
{
    for (auto l: mLegends)
        delete l;
    mLegends.clear();
    for (const auto &s: mSeriesHash.keys())
        removeSeries(s);

    auto chart = mChart->chart();
    if (mAxisX)
    {
        chart->removeAxis(mAxisX);
        delete mAxisX;
    }
    if (mAxisY)
    {
        chart->removeAxis(mAxisY);
        delete mAxisY;
    }
}

void AbstractChartWidget::removeSeries(const QString &category)
{
    if (!mSeriesHash.contains(category))
        return;

    if (mLegends.contains(category))
        delete mLegends.take(category);

    auto chart = mChart->chart();
    auto s = mSeriesHash.take(category);
    if (s.series)
    {
        chart->removeSeries(s.series);
        delete s.series;
    }
}

AbstractChartWidget::Duration AbstractChartWidget::duration() const
{
    return mDuration;
}

void AbstractChartWidget::setDuration(Duration newDuration)
{
    mDuration = newDuration;
}

QDateTime AbstractChartWidget::endDate() const
{
    return mEndDate;
}

void AbstractChartWidget::setEndDate(const QDateTime &newEndDate)
{
    mEndDate = newEndDate;
}

QDateTime AbstractChartWidget::startDate() const
{
    return mStartDate;
}

void AbstractChartWidget::setStartDate(const QDateTime &newStartDate)
{
    mStartDate = newStartDate;
}
