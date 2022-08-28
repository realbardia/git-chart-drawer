#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QDateEdit>
#include <QMainWindow>

#include "commitchartwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    CommitChartWidget::DataType dataType() const;
    void setDataType(CommitChartWidget::DataType newDataType);
    void setDataType(const QString &dataType);
    QStringList dataTypes() const;

    CommitChartWidget::ViewType viewType() const;
    void setViewType(CommitChartWidget::ViewType newViewType);
    void setViewType(const QString &viewType);
    QStringList viewTypes() const;

    AbstractChartWidget::Duration duration() const;
    void setDuration(AbstractChartWidget::Duration newDuration);
    void setDuration(const QString &duration);
    QStringList durations() const;

Q_SIGNALS:
    void finished();

public Q_SLOTS:
    void reloadAll(bool force = false);
    bool saveTo(const QString &path, int w = 2500);
    bool saveJson(const QString &path);
    bool saveCSV(const QString &path);
    bool addPath(const QString &path);

private Q_SLOTS:
    void on_actionAddProject_triggered();
    void on_chart_loading(bool state, qint32 done, qint32 total);
    void on_applyBtn_clicked();
    void on_actionSave_triggered();

protected:
    void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

private:
    Ui::MainWindow *ui;
    bool mBlockReloading = false;
    QDateEdit *mStartDate;
    QDateEdit *mEndDate;
};

#endif // MAINWINDOW_H
