#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QComboBox>
#include <QVBoxLayout>
#include "ReconstructionInspectorDialog.h"

ReconstructionInspectorDialog::ReconstructionInspectorDialog(SLAMReconstructionPtr reconstr, QWidget* parent) : QDialog(parent)
{
    mReconstruction = reconstr;
    mViewer = new ViewerWidget(reconstr);

    mSegmentsWidget = new QComboBox();
    for(int i=0; i<mReconstruction->segments.size(); i++)
    {
        const QString name = "Segment " + QString::number(i);
        mSegmentsWidget->addItem(name, i);
    }

    QToolBar* tb = new QToolBar();
    tb->addWidget(mSegmentsWidget);
    QAction* aMapPoints = tb->addAction("MapPoints");
    QAction* aDensePoints = tb->addAction("DensePoints");
    QAction* aRigs = tb->addAction("Rigs");
    QAction* aTrajectory = tb->addAction("Trajectory");
    tb->addSeparator();
    QAction* aExport = tb->addAction("Export");

    aMapPoints->setCheckable(true);
    aDensePoints->setCheckable(true);
    aRigs->setCheckable(true);
    aTrajectory->setCheckable(true);

    aMapPoints->setChecked(true);
    aDensePoints->setChecked(true);
    aRigs->setChecked(true);
    aTrajectory->setChecked(true);

    connect(aTrajectory, SIGNAL(toggled(bool)), mViewer, SLOT(showTrajectory(bool)));
    connect(aRigs, SIGNAL(toggled(bool)), mViewer, SLOT(showRigs(bool)));
    connect(aDensePoints, SIGNAL(toggled(bool)), mViewer, SLOT(showDensePoints(bool)));
    connect(aMapPoints, SIGNAL(toggled(bool)), mViewer, SLOT(showMapPoints(bool)));
    connect(aExport, SIGNAL(triggered()), this, SLOT(onExport()));
    connect(mSegmentsWidget, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectSegment()));

    /*
    mInformation = new QLabel();
    mInformation->setText("N/A");

    QStatusBar* sb = new QStatusBar();
    sb->addPermanentWidget(mInformation);
    */

    QVBoxLayout* lay = new QVBoxLayout();
    lay->addWidget(tb, 0);
    lay->addWidget(mViewer, 1);
    //lay->addWidget(sb, 0);

    setWindowTitle("Reconstruction Visualization");
    setLayout(lay);

    mViewer->buildScene();
    onSelectSegment();
}

void ReconstructionInspectorDialog::onSelectSegment()
{
    const QVariant v = mSegmentsWidget->currentData();

    if(v.isValid())
    {
        const int seg = v.toInt();

        /*
        if( 0 <= seg && seg < mReconstruction->segments.size() )
        {
            QString txt;
            txt += QString::number(mReconstruction->segments[seg].frames.size()) + " frames.";
            mInformation->setText(txt);
        }
        else
        {
            mInformation->setText("N/A");
        }
        */

        mViewer->showSegment(seg);
    }
}

void ReconstructionInspectorDialog::onExport()
{
    QMessageBox::critical(this, "Error", "Not implemented!");
}

