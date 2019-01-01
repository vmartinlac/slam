#pragma once

#include <memory>
#include "CameraCalibrationData.h"
#include "StereoRigCalibrationData.h"
#include "VideoReader.h"
#include "SLAMDataStructures.h"
#include "SLAMProject.h"


class SLAMSystem
{
public:

    SLAMSystem();
    ~SLAMSystem();

    void run();

protected:

    bool initialize();
    void finalize();
    void printWelcomeMessage();
    void processLastFrame();
    void assignMapPointIds();
    void freeOldImages(int num_to_keep);

protected:

    int mOptionFirst;
    int mOptionCount;
    std::string mOptionName;

    SLAMProjectPtr mProject;

    SLAMModuleOpticalFlowPtr mModuleOpticalFlow;
    SLAMModuleAlignmentPtr mModuleAlignment;

    SLAMModuleFeaturesPtr mModuleFeatures;
    SLAMModuleStereoMatcherPtr mModuleStereoMatcher;
    SLAMModuleTriangulationPtr mModuleTriangulation;

    SLAMModuleDenseReconstructionPtr mModuleDenseReconstruction;

    ReconstructionPtr mReconstruction;
};

typedef std::shared_ptr<SLAMSystem> SLAMSystemPtr;
