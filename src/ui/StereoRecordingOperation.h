
#pragma once

#include <QTime>
#include <QDir>
#include <fstream>
#include <iostream>
#include "Camera.h"
#include "Operation.h"

class StereoRecordingOperation : public Operation
{
public:

    StereoRecordingOperation();

    ~StereoRecordingOperation() override;

    bool before() override;
    bool step() override;
    void after() override;

public:

    QDir mOutputDirectory;
    CameraPtr mCamera;
    int mMaxFrameRate;

protected:

    QTime mClock;
    int mNumFrames;
    std::ofstream mOutputCSV;
};

