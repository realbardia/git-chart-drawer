#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fonts/material/materialicons.h"

#include <QFileDialog>
#include <QLabel>
#include <QSettings>
#include <QMessageBox>
#include <QToolButton>
#include <QDesktopServices>

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

void MainWindow::on_actionAddProject_triggered()
{
    QSettings settings;
    auto path = QFileDialog::getExistingDirectory(this, tr("Please select a git directory"), settings.value("MainWindow/last_directory", QDir::homePath()).toString());
    if (path.isEmpty())
        return;

    QFileInfo inf(path);
    if (!inf.exists())
    {
        QMessageBox::critical(this, tr("Not found"), tr("Directory not found!"));
        return;
    }

    settings.setValue("MainWindow/last_directory", inf.path());

    if (!QFileInfo::exists(path + "/.git"))
    {
        QMessageBox::critical(this, tr("Bad repository"), tr("This is not a git repository. Please select a git repository folder."));
        return;
    }

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
    layout->addWidget(new QLabel(inf.fileName()));
    layout->addStretch();
    layout->addWidget(delBtn);

    ui->listWidget->setItemWidget(item, wgt);
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

