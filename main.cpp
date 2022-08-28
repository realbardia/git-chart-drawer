#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFontDatabase>
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    if (argc == 1)
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    else
        qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication app(argc, argv);
    app.setApplicationName("Git chart drawer");
    app.setOrganizationName("Aseman");
    app.setOrganizationDomain("io.aseman");
    app.setApplicationVersion("0.1.0");
    app.setWindowIcon(QIcon(":/icons/git-chart-drawer.png"));

    QFontDatabase::addApplicationFont(":/fonts/material/MaterialIcons-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/material/materialdesignicons-webfont.ttf");

    MainWindow win;

    if (argc > 1)
    {
        QCommandLineParser parser;
        parser.setApplicationDescription("Application to create charts from you git repository activity.");
        parser.addHelpOption();
        parser.addVersionOption();

        QCommandLineOption inputOption(QStringList() << "i" << "input", QStringLiteral("Input git directory. (Required)"), "dir");
        parser.addOption(inputOption);

        QCommandLineOption destOption(QStringList() << "o" << "output", QStringLiteral("Output file path. (Required)"), "file");
        parser.addOption(destOption);

        QCommandLineOption formatOption(QStringList() << "f" << "format", QStringLiteral("Output format."), "json|image");
        parser.addOption(formatOption);

        QCommandLineOption widthOption(QStringList() << "w" << "width", QStringLiteral("Width of output image"), "pixels");
        parser.addOption(widthOption);

        QCommandLineOption dataOption(QStringList() << "data", QStringLiteral("Type of data to analize."), win.dataTypes().join('|'), "changes");
        parser.addOption(dataOption);

        QCommandLineOption viewOption(QStringList() << "view", QStringLiteral("View mode."), win.viewTypes().join('|'), "overall");
        parser.addOption(viewOption);

        QCommandLineOption durationOption(QStringList() << "duration", QStringLiteral("Type of duration"), win.durations().join('|'), "weekly");
        parser.addOption(durationOption);

        parser.process(app);

        if (!parser.isSet(inputOption) || !parser.isSet(destOption))
        {
            parser.showHelp();
            return 0;
        }

        auto input = parser.value(inputOption);
        auto dest = parser.value(destOption);
        auto format = parser.value(formatOption);
        int width = parser.isSet(widthOption)? parser.value(widthOption).toInt() : 2500;
        auto view = parser.value(viewOption);
        auto data = parser.value(dataOption);
        auto duration = parser.value(durationOption);

        if (!QDir(input).exists())
        {
            qDebug() << "Can't open input file: No such directory.";
            return 0;
        }

        if (format.isEmpty())
        {
            if (dest.right(5).toLower() == QStringLiteral(".json"))
                format = QStringLiteral("json");
            if (dest.right(4).toLower() == QStringLiteral(".csv"))
                format = QStringLiteral("csv");
        }

        if (duration.length()) win.setDuration(duration);
        if (data.length()) win.setDataType(data);
        if (view.length()) win.setViewType(view);
        win.connect(&win, &MainWindow::finished, &app, [&win, dest, width, format]{
            QTimer::singleShot(10, &win, [&win, dest, width, format](){
                if (format == "json")
                    win.saveJson(dest);
                else
                if (format == "csv")
                    win.saveCSV(dest);
                else
                    win.saveTo(dest, width);
                qApp->quit();
            });
        });
        win.addPath(input);
    }

    win.show();

    return app.exec();
}
