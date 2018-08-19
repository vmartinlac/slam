#include <QToolBar>
#include <QIcon>
#include <QApplication>
#include <QKeySequence>
#include <QSplitter>
#include <QLabel>
#include <QMessageBox>
#include "ViewerWidget.h"
#include "MainWindow.h"
#include "ParametersDialog.h"
#include "VideoInputDialog.h"

MainWindow::MainWindow(SLAMEngine* slam, QWidget* parent) :
    QMainWindow(parent),
    m_slam(slam)
{
    QToolBar* tb = addToolBar("Toolbar");
    tb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    m_a_start = tb->addAction("Start");
    m_a_stop = tb->addAction("Stop");
    tb->addSeparator();
    m_a_video = tb->addAction("Video");
    m_a_parameters = tb->addAction("Parameters");
    tb->addSeparator();
    QAction* a_about = tb->addAction("About");
    QAction* a_quit = tb->addAction("Quit");

    connect(m_a_parameters, SIGNAL(triggered()), this, SLOT(ask_slam_parameters()));
    connect(m_a_video, SIGNAL(triggered()), this, SLOT(ask_video_input()));

    connect(m_a_start, SIGNAL(triggered()), this, SLOT(start_slam()));
    connect(m_a_stop, SIGNAL(triggered()), this, SLOT(stop_slam()));
    connect(a_about, SIGNAL(triggered()), this, SLOT(about()));

    connect(m_slam, SIGNAL(started()), this, SLOT(slam_started()));
    connect(m_slam, SIGNAL(finished()), this, SLOT(slam_stopped()));

    connect(a_quit, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

    m_a_start->setShortcut(QKeySequence("Ctrl+R"));
    m_a_stop->setShortcut(QKeySequence("Ctrl+S"));
    m_a_parameters->setShortcut(QKeySequence("Ctrl+P"));
    m_a_video->setShortcut(QKeySequence("Ctrl+V"));
    a_quit->setShortcut(QKeySequence("Ctrl+Q"));

    m_a_start->setIcon(QIcon::fromTheme("media-playback-start"));
    m_a_stop->setIcon(QIcon::fromTheme("media-playback-stop"));
    a_about->setIcon(QIcon::fromTheme("help-about"));
    a_quit->setIcon(QIcon::fromTheme("application-exit"));
    m_a_parameters->setIcon(QIcon::fromTheme("document-properties"));
    m_a_video->setIcon(QIcon::fromTheme("camera-video"));

    slam_stopped();

    m_viewer = new ViewerWidget( m_slam->getOutput() );
    m_stats = new StatsWidget( m_slam->getOutput() );
    m_video = new VideoWidget( m_slam->getOutput() );

    QSplitter* outer_splitter = new QSplitter();
    QSplitter* inner_splitter = new QSplitter();

    outer_splitter->setOrientation(Qt::Horizontal);
    outer_splitter->setChildrenCollapsible(false);
    inner_splitter->setOrientation(Qt::Vertical);
    inner_splitter->setChildrenCollapsible(false);

    outer_splitter->addWidget(m_viewer);
    outer_splitter->addWidget(inner_splitter);
    inner_splitter->addWidget(m_video);
    inner_splitter->addWidget(m_stats);

    setCentralWidget(outer_splitter);
    setWindowTitle("SLAM Demo");
    resize(800, 600);

    m_slam_parameters.loadFromSettings();
}

void MainWindow::ask_slam_parameters()
{
    const bool ret = ParametersDialog::ask(this, m_slam_parameters);

    if( ret )
    {
        m_slam_parameters.saveToSettings();
    }
}

void MainWindow::about()
{
    QMessageBox::information(this, "About", "Victor Martin Lac 2018");
}

void MainWindow::start_slam()
{
    if( m_slam->isRunning() == false )
    {
        m_a_parameters->setEnabled(false);

        m_slam->setParameters(m_slam_parameters);
        m_slam->start();
    }
}

void MainWindow::stop_slam()
{
    if( m_slam->isRunning() )
    {
        m_slam->requestInterruption();
    }
}

void MainWindow::slam_started()
{
    m_a_start->setEnabled(false);
    //m_a_start->setVisible(false);
    m_a_stop->setEnabled(true);
    m_a_parameters->setEnabled(false);
    //m_a_stop->setVisible(true);
}

void MainWindow::slam_stopped()
{
    m_a_start->setEnabled(true);
    //m_a_start->setVisible(true);
    m_a_stop->setEnabled(false);
    m_a_parameters->setEnabled(true);
    //m_a_stop->setVisible(false);
}

void MainWindow::ask_video_input()
{
    VideoInputDialog* dlg = new VideoInputDialog(this);
    dlg->exec();
    delete dlg;
}

