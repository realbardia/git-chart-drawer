#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fonts/material/materialicons.h"

#include <QFileDialog>
#include <QLabel>
#include <QSettings>
#include <QMessageBox>
#include <QToolButton>
#include <QDesktopServices>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->progressBar->hide();

    mStartDate = new QDateEdit();
    mStartDate->setDisplayFormat("yyyy MMM dd");
    mStartDate->setCalendarPopup(true);
    mStartDate->setEnabled(false);

    mEndDate = new QDateEdit();
    mEndDate->setDisplayFormat("yyyy MMM dd");
    mEndDate->setCalendarPopup(true);
    mEndDate->setEnabled(false);

    connect(mStartDate, &QDateEdit::editingFinished, this, [this](){ reloadAll(); });
    connect(mEndDate, &QDateEdit::editingFinished, this, [this](){ reloadAll(); });

    auto dateWidget = new QWidget;
    auto dateLayout = new QHBoxLayout(dateWidget);
    dateLayout->addStretch();
    dateLayout->addWidget(new QLabel("From:"));
    dateLayout->addWidget(mStartDate);
    dateLayout->addWidget(new QLabel("To:"));
    dateLayout->addWidget(mEndDate);

    ui->toolBar->addWidget(dateWidget);

    auto plt = palette();
    plt.setColor(QPalette::Button, QColor(QStringLiteral("#0d80ec")));
    plt.setColor(QPalette::ButtonText, QColor(QStringLiteral("#ffffff")));

    ui->applyBtn->setPalette(plt);

    splitDockWidget(ui->projectsDock, ui->viewOptionsDock, Qt::Vertical);

    QSettings settings;
    resize(settings.value("MainWindow/size", size()).toSize());
    restoreState(settings.value("MainWindow/state", QByteArray()).toByteArray());
}

MainWindow::~MainWindow()
{
    delete ui;
}

CommitChartWidget::DataType MainWindow::dataType() const
{
    return ui->chart->dataType();
}

void MainWindow::setDataType(CommitChartWidget::DataType newDataType)
{
    ui->chart->setDataType(newDataType);
    ui->data->setCurrentIndex(static_cast<int>(newDataType));
}

void MainWindow::setDataType(const QString &dataType)
{
    for (int i=0; i<ui->data->count(); i++)
        if (ui->data->itemText(i).toLower() == dataType.toLower())
        {
            ui->data->setCurrentIndex(i);
            break;
        }
}

QStringList MainWindow::dataTypes() const
{
    QStringList list;
    for (int i=0; i<ui->data->count(); i++)
        list << ui->data->itemText(i).toLower();
    return list;
}

CommitChartWidget::ViewType MainWindow::viewType() const
{
    return ui->chart->viewType();
}

void MainWindow::setViewType(CommitChartWidget::ViewType newViewType)
{
    ui->chart->setViewType(newViewType);
    ui->view->setCurrentIndex(static_cast<int>(newViewType));
}

void MainWindow::setViewType(const QString &viewType)
{
    for (int i=0; i<ui->view->count(); i++)
        if (ui->view->itemText(i).toLower() == viewType.toLower())
        {
            ui->view->setCurrentIndex(i);
            break;
        }
}

QStringList MainWindow::viewTypes() const
{
    QStringList list;
    for (int i=0; i<ui->view->count(); i++)
        list << ui->view->itemText(i).toLower();
    return list;
}

AbstractChartWidget::Duration MainWindow::duration() const
{
    return ui->chart->duration();
}

void MainWindow::setDuration(AbstractChartWidget::Duration newDuration)
{
    ui->chart->setDuration(newDuration);
    ui->duration->setCurrentIndex(static_cast<int>(newDuration));
}

void MainWindow::setDuration(const QString &duration)
{
    for (int i=0; i<ui->duration->count(); i++)
        if (ui->duration->itemText(i).toLower() == duration.toLower())
        {
            ui->duration->setCurrentIndex(i);
            break;
        }
}

QStringList MainWindow::durations() const
{
    QStringList list;
    for (int i=0; i<ui->duration->count(); i++)
        list << ui->duration->itemText(i).toLower();
    return list;
}

void MainWindow::on_actionAddProject_triggered()
{
    QSettings settings;
    auto path = QFileDialog::getExistingDirectory(this, tr("Please select a git directory"), settings.value("MainWindow/last_directory", QDir::homePath()).toString());
    if (path.isEmpty())
        return;

    QDir inf(path);
    if (!inf.exists())
    {
        QMessageBox::critical(this, tr("Not found"), tr("Directory not found!"));
        return;
    }

    settings.setValue("MainWindow/last_directory", inf.path());

    if (!QDir(path + "/.git").exists())
    {
        QMessageBox::critical(this, tr("Bad repository"), tr("This is not a git repository. Please select a git repository folder."));
        return;
    }

    addPath(path);
}

void MainWindow::on_chart_loading(bool state, qint32 done, qint32 total)
{
    ui->progressBar->setVisible(state);
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(done);

    if (!state)
    {
        mBlockReloading = true;
        mStartDate->setDateTime(ui->chart->minDate());
        mStartDate->setEnabled(true);

        mEndDate->setDateTime(ui->chart->maxDate());
        mEndDate->setEnabled(true);

        Q_EMIT finished();
    }
}

void MainWindow::reloadAll(bool force)
{
    if (mBlockReloading && !force)
        return;

    ui->chart->setSplineMode(ui->chartMode->currentIndex());
    ui->chart->setStartDate(mStartDate->dateTime());
    ui->chart->setEndDate(mEndDate->dateTime());
    ui->chart->setDuration( static_cast<AbstractChartWidget::Duration>(ui->duration->currentIndex()) );
    ui->chart->setViewType( static_cast<CommitChartWidget::ViewType>(ui->view->currentIndex()) );
    ui->chart->setDataType( static_cast<CommitChartWidget::DataType>(ui->data->currentIndex()) );
    ui->chart->reload();
}

bool MainWindow::saveTo(const QString &path, int w)
{
    return ui->chart->saveTo(path, w);
}

bool MainWindow::saveJson(const QString &path)
{
    return ui->chart->saveJson(path);
}

bool MainWindow::saveCSV(const QString &path)
{
    return ui->chart->saveCSV(path);
}

bool MainWindow::addPath(const QString &path)
{
    QDir inf(path);
    if (!inf.exists())
        return false;

    ui->chart->load(path);

    auto item = new QListWidgetItem;

    ui->listWidget->addItem(item);

    auto font = QMainWindow::font();
    font.setFamily(MaterialIcons::mdi_family);

    auto visibleBtn = new QToolButton();
    visibleBtn->setFont(font);
    visibleBtn->setText(MaterialIcons::mdi_eye);
    visibleBtn->setAutoRaise(true);
    visibleBtn->setFixedWidth(22);

    connect(visibleBtn, &QPushButton::clicked, this, [path, visibleBtn, this](){
        auto disabled = visibleBtn->property("disabled").toBool();
        if (disabled)
            ui->chart->load(path);
        else
            ui->chart->remove(path);

        disabled = !disabled;
        visibleBtn->setText(disabled? MaterialIcons::mdi_eye_off : MaterialIcons::mdi_eye);
        visibleBtn->setProperty("disabled", disabled);
    });

    auto delBtn = new QToolButton();
    delBtn->setFont(font);
    delBtn->setText(MaterialIcons::mdi_delete);
    delBtn->setAutoRaise(true);
    delBtn->setFixedWidth(22);

    connect(delBtn, &QPushButton::clicked, this, [path, item, this](){
        ui->chart->remove(path);
        delete item;
    });

    auto wgt = new QWidget;
    auto layout = new QHBoxLayout(wgt);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(visibleBtn);
    layout->addWidget(new QLabel(inf.dirName()));
    layout->addStretch();
    layout->addWidget(delBtn);

    ui->listWidget->setItemWidget(item, wgt);
    return true;
}

void MainWindow::on_applyBtn_clicked()
{
    reloadAll(true);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    QSettings settings;
    settings.setValue("MainWindow/size", size());
    settings.setValue("MainWindow/state", saveState());

    QMainWindow::closeEvent(e);
}

void MainWindow::on_actionSave_triggered()
{
    QSettings settings;
    auto path = QFileDialog::getSaveFileName(this, tr("Save Image"), settings.value("MainWindow/last_save_path", QDir::homePath()).toString(), "Image files (*.png *.jpg)");
    if (path.isEmpty())
        return;

    if (!saveTo(path))
    {
        QMessageBox::critical(this, tr("Save error"), tr("Could not save image."));
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

