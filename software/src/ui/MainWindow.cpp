#include <QMenu>
#include <QKeySequence>
#include <QTreeWidget>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QStatusBar>
#include <QListWidget>
#include <QToolBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QTabWidget>
#include <QMenuBar>
#include <QAction>
#include "MainWindow.h"
#include "RecordingPanel.h"
#include "CalibrationPanel.h"
#include "ReconstructionPanel.h"
#include "AboutDialog.h"
#include "AvailableCameraDialog.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    mProject = new Project(this);

    QMenu* menuProject = menuBar()->addMenu("Project");
    QAction* aNew = menuProject->addAction("New");
    QAction* aOpen = menuProject->addAction("Open");
    QAction* aClear = menuProject->addAction("Clear");
    QAction* aClose = menuProject->addAction("Close");
    QAction* aQuit = menuProject->addAction("Quit");

    QMenu* menuHelp = menuBar()->addMenu("Help");
    QAction* aCameras = menuHelp->addAction("Available cameras");
    QAction* aAbout = menuHelp->addAction("About Salma");

    aNew->setShortcut(QKeySequence("Ctrl+N"));
    aOpen->setShortcut(QKeySequence("Ctrl+O"));
    aClose->setShortcut(QKeySequence("Ctrl+W"));
    aQuit->setShortcut(QKeySequence("Ctrl+Q"));

    connect(aNew, SIGNAL(triggered()), this, SLOT(newProject()));
    connect(aOpen, SIGNAL(triggered()), this, SLOT(openProject()));
    connect(aClose, SIGNAL(triggered()), this, SLOT(closeProject()));
    connect(aQuit, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));
    connect(aClear, SIGNAL(triggered()), this, SLOT(clearProject()));
    connect(aCameras, SIGNAL(triggered()), this, SLOT(showAvailableCameras()));
    connect(aAbout, SIGNAL(triggered()), this, SLOT(about()));

    QTabWidget* tab = new QTabWidget();
    tab->addTab(new RecordingPanel(mProject), "Recording");
    tab->addTab(new CalibrationPanel(mProject), "Calibration");
    tab->addTab(new ReconstructionPanel(mProject), "Reconstruction");
    setCentralWidget(tab);

    //statusBar()->showMessage("SALMA v1.0");

    setWindowTitle("SALMA");
    resize(640, 480);
}

void MainWindow::about()
{
    AboutDialog* dlg = new AboutDialog(this);
    dlg->exec();
    delete dlg;
}

void MainWindow::showAvailableCameras()
{
    AvailableCameraDialog* dlg = new AvailableCameraDialog(this);
    dlg->exec();
    delete dlg;
}

void MainWindow::clearProject()
{
    if( mProject->isOpen() == false )
    {
        QMessageBox::critical(this, "Error", "No project is open!");
    }
    else
    {
        const int ret = QMessageBox::question(this, "Clear Project", "Do you really want to clear project's content?", QMessageBox::Yes|QMessageBox::No);

        if(ret == QMessageBox::Yes)
        {
            bool ok = true;
            
            if(ok)
            {
                ok = mProject->clear();
            }

            if(ok == false)
            {
                QMessageBox::critical(this, "Error", "An error happened while clearing the project!");
            }
        }
    }
}

void MainWindow::openProject()
{
    QString ret = QFileDialog::getExistingDirectory(this, "Open project");

    if( ret.isEmpty() == false )
    {
        const bool ok = mProject->open(ret);

        if(ok == false)
        {
            QMessageBox::critical(this, "Error", "Could not load project!");
        }
    }
}

void MainWindow::closeProject()
{
    mProject->close();
}

void MainWindow::newProject()
{
    QString ret = QFileDialog::getExistingDirectory(this, "New project");

    if( ret.isEmpty() == false )
    {
        const bool ok = mProject->create(ret);

        if(ok == false)
        {
            QMessageBox::critical(this, "Error", "Could not create new project!");
        }
    }
}

void MainWindow::loadProjectGivenOnCommandLine(const QString& path)
{
    const bool ok = mProject->open(path);

    if(ok == false)
    {
        QMessageBox::critical(this, "Error", "Could not load project!");
    }
}

