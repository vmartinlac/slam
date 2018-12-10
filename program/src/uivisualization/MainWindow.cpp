#include <QIcon>
#include <QAction>
#include <QTabWidget>
#include <QToolBar>
#include "OpenReconstructionDialog.h"
#include "VisualizationSettingsDialog.h"
#include "ExportPointCloudDialog.h"
#include "AboutDialog.h"
#include "MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
{
    QToolBar* tb = addToolBar("ToolBar");

    QAction* a_open_reconstruction = tb->addAction("Open Reconstruction");
    QAction* a_visualization_settings = tb->addAction("Visualization Settings");
    QAction* a_export_point_cloud = tb->addAction("Export Point Cloud");
    QAction* a_about = tb->addAction("About");

    /*
    a_open_reconstruction->setIcon(QIcon::fromTheme("document-open"));
    a_open_reconstruction->setIcon(QIcon::fromTheme("emblem-system"));
    a_export_point_cloud->setIcon(QIcon::fromTheme("document-save"));
    a_about->setIcon(QIcon::fromTheme("help-about"));
    */

    connect(a_open_reconstruction, SIGNAL(triggered()), this, SLOT(openReconstruction()));
    connect(a_visualization_settings, SIGNAL(triggered()), this, SLOT(visualizationSettings()));
    connect(a_export_point_cloud, SIGNAL(triggered()), this, SLOT(exportPointCloud()));
    connect(a_about, SIGNAL(triggered()), this, SLOT(about()));

    ViewerWidget* w = new ViewerWidget();

    setCentralWidget(w);
    setWindowTitle("Salma Visualization");
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::openReconstruction()
{
    OpenReconstructionDialog* dlg = new OpenReconstructionDialog(this);
    const int ret = dlg->exec();

    if( ret == QDialog::Accepted )
    {
        mFrames = dlg->getReconstruction();
    }

    delete dlg;
}

void MainWindow::visualizationSettings()
{
    VisualizationSettingsDialog* dlg = new VisualizationSettingsDialog(this);
    dlg->exec();
    delete dlg;
}

void MainWindow::exportPointCloud()
{
    ExportPointCloudDialog* dlg = new ExportPointCloudDialog(this);
    dlg->exec();
    delete dlg;
}

void MainWindow::about()
{
    AboutDialog* dlg = new AboutDialog(this);
    dlg->exec();
    delete dlg;
}

