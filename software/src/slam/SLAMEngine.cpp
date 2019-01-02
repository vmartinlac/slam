#include "SLAMModuleOpticalFlow.h"
#include "SLAMModuleAlignment.h"
#include "SLAMModuleFeatures.h"
#include "SLAMModuleStereoMatcher.h"
#include "SLAMModuleTriangulation.h"
#include "SLAMModuleDenseReconstruction.h"
#include "SLAMEngine.h"

SLAMEngine::SLAMEngine()
{
}

SLAMEngine::~SLAMEngine()
{
}

bool SLAMEngine::initialize(
    StereoRigCalibrationDataPtr calibration,
    SLAMConfigurationPtr configuration)
{
    mFrameCount = 0;

    mContext.reset(new SLAMContext);
    mContext->calibration = calibration;
    mContext->configuration = configuration;
    mContext->reconstruction.reset(new SLAMReconstruction());
    mContext->debug.reset(new SLAMDebug( mContext->configuration ));

    mModuleOpticalFlow.reset(new SLAMModuleOpticalFlow(mContext));
    mModuleAlignment.reset(new SLAMModuleAlignment(mContext));
    mModuleFeatures.reset(new SLAMModuleFeatures(mContext));
    mModuleStereoMatcher.reset(new SLAMModuleStereoMatcher(mContext));
    mModuleTriangulation.reset(new SLAMModuleTriangulation(mContext));
    mModuleDenseReconstruction.reset(new SLAMModuleDenseReconstruction(mContext));

    bool ok = true;

    ok = ok && mContext->debug->init();

    ok = ok && mModuleOpticalFlow->init();
    ok = ok && mModuleAlignment->init();
    ok = ok && mModuleFeatures->init();
    ok = ok && mModuleStereoMatcher->init();
    ok = ok && mModuleTriangulation->init();
    ok = ok && mModuleDenseReconstruction->init();

    return ok;
}

bool SLAMEngine::processFrame(Image& image)
{
    if( image.isValid() )
    {
        if( image.getNumberOfFrames() != 2 ) throw std::runtime_error("incorrect number of frames");

        SLAMFramePtr curr_frame(new SLAMFrame());

        curr_frame->id = mFrameCount;
        curr_frame->timestamp = image.getTimestamp();
        curr_frame->views[0].image = image.getFrame(0);
        curr_frame->views[1].image = image.getFrame(1);

        mContext->reconstruction->frames.push_back(curr_frame);

        std::cout << "PROCESSING FRAME " << curr_frame->id << std::endl;

        {
            std::cout << "   OPTICAL FLOW" << std::endl;
            (*mModuleOpticalFlow)();
            std::cout << "      Number of projections on left view: " << curr_frame->views[0].projections.size() << std::endl;
            std::cout << "      Number of projections on right view: " << curr_frame->views[1].projections.size() << std::endl;
        }

        {
            std::cout << "   ALIGNMENT" << std::endl;
            (*mModuleAlignment)();
            std::cout << "      Alignment status: " << ( (curr_frame->aligned_wrt_previous_frame) ? "ALIGNED" : "NOT ALIGNED" ) << std::endl;
            std::cout << "      Position: " << curr_frame->frame_to_world.translation().transpose() << std::endl;
            std::cout << "      Attitude: " << curr_frame->frame_to_world.unit_quaternion().coeffs().transpose() << std::endl;
        }

        {
            std::cout << "   FEATURES DETECTION" << std::endl;
            (*mModuleFeatures)();
            std::cout << "      Num keypoints on left view: " << curr_frame->views[0].keypoints.size() << std::endl;
            std::cout << "      Num keypoints on right view: " << curr_frame->views[1].keypoints.size() << std::endl;
        }

        {
            std::cout << "   STEREO MATCHING" << std::endl;
            (*mModuleStereoMatcher)();
            std::cout << "      Number of stereo matches: " << curr_frame->stereo_matches.size() << std::endl;
        }

        {
            std::cout << "   TRIANGULATION" << std::endl;
            (*mModuleTriangulation)();
            //std::cout << "      Number of new mappoints: " << mModuleTriangulation->getNumberOfNewMapPoints() << std::endl;
        }

        /*
        {
            std::cout << "   DENSE RECONSTRUCTION" << std::endl;
            (*mModuleDenseReconstruction)();
        }
        */

        mFrameCount++;
    }

    return true;
}

bool SLAMEngine::finalize(SLAMReconstructionPtr& reconstruction)
{
    reconstruction = std::move(mContext->reconstruction);

    mContext.reset();
    mModuleOpticalFlow.reset();
    mModuleAlignment.reset();
    mModuleFeatures.reset();
    mModuleStereoMatcher.reset();
    mModuleTriangulation.reset();
    mModuleDenseReconstruction.reset();

    return true;
}

