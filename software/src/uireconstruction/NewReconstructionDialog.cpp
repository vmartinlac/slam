#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QSettings>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include "NewReconstructionDialog.h"
#include "VideoSystem.h"
#include "ReconstructionOperation.h"
#include "SLAMConfiguration.h"

NewReconstructionDialog::NewReconstructionDialog(Project* proj, QWidget* parent) : NewOperationDialog(proj, parent)
{
    QTabWidget* tab = new QTabWidget();
    tab->addTab(createNameAndInputTab(), "Name and Input");
    tab->addTab(createConfigurationTab(), "Configuration");

    QPushButton* btnok = new QPushButton("OK");
    QPushButton* btncancel = new QPushButton("Cancel");

    QHBoxLayout* hlay = new QHBoxLayout();
    hlay->addWidget(btnok);
    hlay->addWidget(btncancel);

    QVBoxLayout* vlay = new QVBoxLayout();
    vlay->addWidget(tab);
    vlay->addLayout(hlay);

    setLayout(vlay);
    setWindowTitle("New Reconstruction");

    connect(btnok, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btncancel, SIGNAL(clicked()), this, SLOT(reject()));
}

QWidget* NewReconstructionDialog::createNameAndInputTab()
{
    mName = new QLineEdit();
    mRecording = new RecordingListWidget(project());
    mCalibration = new CalibrationListWidget(project());

    mName->setText("new reconstruction");

    QFormLayout* form = new QFormLayout();
    form->addRow("Name:", mName);
    form->addRow("Recording:", mRecording);
    form->addRow("Rig calibration:", mCalibration);

    QWidget* ret = new QWidget();
    ret->setLayout(form);

    return ret;
}

QWidget* NewReconstructionDialog::createConfigurationTab()
{
    mCheckDenseReconstruction = new QCheckBox("Produce dense point cloud");
    mCheckDebugR = new QCheckBox("Debug R module");
    mCheckDebugF = new QCheckBox("Debug F module");
    mCheckDebugTM = new QCheckBox("Debug TM module");
    mCheckDebugA = new QCheckBox("Debug A module");
    mCheckDebugKFS = new QCheckBox("Debug KFS module");
    mCheckDebugLBA = new QCheckBox("Debug LBA module");
    mCheckDebugSM = new QCheckBox("Debug SM module");
    mCheckDebugT = new QCheckBox("Debug T module");
    mCheckDebugDR = new QCheckBox("Debug DR module");

    QFormLayout* lay = new QFormLayout();
    lay->addWidget( mCheckDenseReconstruction );
    lay->addWidget( mCheckDebugR );
    lay->addWidget( mCheckDebugF );
    lay->addWidget( mCheckDebugTM );
    lay->addWidget( mCheckDebugA );
    lay->addWidget( mCheckDebugKFS );
    lay->addWidget( mCheckDebugLBA );
    lay->addWidget( mCheckDebugSM );
    lay->addWidget( mCheckDebugT );
    lay->addWidget( mCheckDebugDR );

    QWidget* ret = new QWidget();
    ret->setLayout(lay);

    return ret;
}

void NewReconstructionDialog::accept()
{
    QString name;
    int calibration_id = -1;
    int recording_id = -1;
    StereoRigCalibrationPtr calibration;
    RecordingHeaderPtr recording;

    OperationPtr op;
    bool ok = true;
    const char* err = "";

    if(ok)
    {
        name = mName->text();
        ok = (name.isEmpty() == false);
        err = "Incorrect name!";
    }

    if(ok)
    {
        calibration_id = mCalibration->getCalibrationId();
        ok = (calibration_id >= 0);
        err = "Incorrect rig calibration!";
    }

    if(ok)
    {
        recording_id = mRecording->getRecordingId();
        ok = (recording_id >= 0);
        err = "Incorrect recording!";
    }

    if(ok)
    {
        ok = project()->loadCalibration(calibration_id, calibration);
        err = "Could not load rig calibration!";
    }

    if(ok)
    {
        ok = project()->loadRecording(recording_id, recording);
        err = "Could not load recording!";
    }

    if(ok)
    {
        ok = (recording->num_views() == 2);
        err = "Recording must be stereo!";
    }

    if(ok)
    {
        ReconstructionOperation* myop = new ReconstructionOperation();
        myop->mReconstructionName = mName->text().toStdString();
        myop->mCalibration = calibration;
        myop->mRecordingHeader = recording;

        myop->mFrameFirst = 0;
        myop->mFrameStride = 1;
        myop->mFrameLast = recording->num_frames() - 1;

        myop->mConfiguration.reset(new SLAMConfiguration());

        myop->mConfiguration->dense_reconstruction.enabled = mCheckDenseReconstruction->isChecked();

        myop->mConfiguration->rectification.debug = mCheckDebugR->isChecked();
        myop->mConfiguration->features.debug = mCheckDebugF->isChecked();
        myop->mConfiguration->temporal_matcher.debug = mCheckDebugTM->isChecked();
        myop->mConfiguration->alignment.debug = mCheckDebugA->isChecked();
        myop->mConfiguration->kfs.debug = mCheckDebugKFS->isChecked();
        myop->mConfiguration->lba.debug = mCheckDebugLBA->isChecked();
        myop->mConfiguration->stereo_matcher.debug = mCheckDebugSM->isChecked();
        myop->mConfiguration->triangulation.debug = mCheckDebugT->isChecked();
        myop->mConfiguration->dense_reconstruction.debug = mCheckDebugDR->isChecked();

        op.reset(myop);
    }

    if(ok)
    {
        setOperation(op);
        QDialog::accept();
    }
    else
    {
        QMessageBox::critical(this, "Error", err);
    }
}

