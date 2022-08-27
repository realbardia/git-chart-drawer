#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QDateEdit>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

Q_SIGNALS:
    void finished();

public Q_SLOTS:
    void reloadAll(bool force = false);
    bool saveTo(const QString &path, int w = 2500);

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
