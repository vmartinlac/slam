#pragma once

#include <QCheckBox>
#include <QSpinBox>
#include "OperationParametersWidget.h"
#include "CameraList.h"
#include "PathWidget.h"

class StereoRecordingParametersWidget : public OperationParametersWidget
{
    Q_OBJECT

public:

    StereoRecordingParametersWidget(QWidget* parent=nullptr);

    OperationPtr getOperation() override;

    QString name() override;

protected:

    CameraList* mLeftCamera;
    CameraList* mRightCamera;
    PathWidget* mOutputPath;
    QSpinBox* mMaxFrameRate;
    QCheckBox* mVisualizationOnly;
};
