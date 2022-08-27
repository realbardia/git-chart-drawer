#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QFontDatabase>
#include <QDebug>

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

        QApplication app(argc, argv);
        app.setApplicationName("Git chart drawer");
        app.setOrganizationName("Aseman");
        app.setOrganizationDomain("io.aseman");
        app.setApplicationVersion("0.1.0");

        QFontDatabase::addApplicationFont(":/fonts/material/MaterialIcons-Regular.ttf");
        QFontDatabase::addApplicationFont(":/fonts/material/materialdesignicons-webfont.ttf");

        MainWindow win;
        win.show();

        return app.exec();
    }
    else
    {
        qputenv("QT_QPA_PLATFORM", "offscreen");

        QApplication app(argc, argv);
        app.setApplicationName("Git chart drawer");
        app.setOrganizationName("Aseman");
        app.setOrganizationDomain("io.aseman");
        app.setApplicationVersion("0.1.0");


        QCommandLineParser parser;
        parser.setApplicationDescription("Application to create charts from you git repository activity.");
        parser.addHelpOption();
        parser.addVersionOption();

        QCommandLineOption inputOption(QStringList() << "i" << "input", QStringLiteral("Input git directory. (Required)"), "dir");
        parser.addOption(inputOption);

        QCommandLineOption destOption(QStringList() << "o" << "output", QStringLiteral("Output file path. (Required)"), "file");
        parser.addOption(destOption);

        QCommandLineOption formatOption(QStringList() << "f" << "format", QStringLiteral("Output format."), "csv|image");
        parser.addOption(formatOption);

        QCommandLineOption widthOption(QStringList() << "w" << "width", QStringLiteral("Width of output image"), "pixels");
        parser.addOption(widthOption);

        QCommandLineOption dataOption(QStringList() << "data", QStringLiteral("Type of data to analize."), "changes|files", "changes");
        parser.addOption(dataOption);

        QCommandLineOption viewOption(QStringList() << "view", QStringLiteral("View mode."), "overall|commiters", "overall");
        parser.addOption(viewOption);

        QCommandLineOption durationOption(QStringList() << "duration", QStringLiteral("Type of duration"), "daily|weekly|monthly|yearly", "weekly");
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
        auto duration = parser.isSet(durationOption);

        if (!QFileInfo::exists(input))
        {
            qDebug() << "Can't open input file: No such file or directory.";
            return 0;
        }

        MainWindow win;
        win.connect(&win, &MainWindow::finished, &app, &QApplication::quit);
        win.saveTo(dest, width);

        return app.exec();
    }
}
