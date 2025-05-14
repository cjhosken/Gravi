///
/// @file main.cpp
/// @brief The entry point to the application

#include <iostream>

#include <QApplication>
#include <QString>
#include <QDir>

#include "gravityWell.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // General Application Settings
    QApplication app(argc, argv);
    QApplication::setApplicationName("Gravi");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("Christopher Hosken");
    QApplication::setOrganizationDomain("cjhosken.github.io");

    // Global OpenGL Settings.
    QSurfaceFormat format;
    format.setVersion(4, 5);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    QSurfaceFormat::setDefaultFormat(format);

    // Setting up PLUGINPATH and PYTHONPATH to ensure the application works correctly.
    QDir rootDir(QCoreApplication::applicationDirPath() + "/../");
    QDir pluginDir(rootDir.absolutePath() + "/plugin/usd");
    QDir pythonDir(rootDir.absolutePath() + "/lib/python");

    app.setWindowIcon(QIcon(":/icons/logo"));

    if (pluginDir.exists()) {
        qputenv("PXR_PLUGINPATH_NAME", pluginDir.absolutePath().toUtf8());
    } else {
        qCritical() << "Failed to find USD plugin directory";
    }

    if (pythonDir.exists()) {
        qputenv("PYTHONPATH", pythonDir.absolutePath().toUtf8());
    } else {
        qCritical() << "Failed to find USD plugin directory";
    }

    // Creating the Main Window and launching the app.
    MainWindow mainWindow;
    mainWindow.show();
    return QApplication::exec();
}