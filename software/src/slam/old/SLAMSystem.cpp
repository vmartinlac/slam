#include <QCoreApplication>
#include <QCommandLineParser>
#include <Eigen/Eigen>
#include <opencv2/core/version.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <thread>
#include "VideoSystem.h"
#include "Image.h"
#include "BuildInfo.h"
#include "SLAMSystem.h"
#include "Debug.h"

SLAMSystem::SLAMSystem()
{
    mOptionFirst = 0;
    mOptionCount = 0;
}

SLAMSystem::~SLAMSystem()
{
}

bool SLAMSystem::initialize()
{
    QCommandLineParser parser;
    bool go_on = true;

    mOptionFirst = 0;
    mOptionCount = -1;
    mOptionName = "myreconstruction";

    {
        parser.setApplicationDescription("Perform SLAM on given video. Writen by Victor Martin Lac in 2018.");
        parser.addHelpOption();
        parser.addPositionalArgument("PROJECT_PATH", "Path to project root directory");

        parser.addOption(QCommandLineOption("first", "Index of the first frame to be processed", "FIRST_FRAME", "0"));
        parser.addOption(QCommandLineOption("count", "Number of frames to be processed", "NUMBER_OF_FRAMES", "-1"));
        parser.addOption(QCommandLineOption("recname", "Name of saved reconstruction", "NAME", "myreconstruction"));
    }

    if(go_on)
    {
        go_on = parser.parse(QCoreApplication::arguments());

        if(go_on == false)
        {
            std::cout << "Incorrect command line!" << std::endl;
            std::cout << parser.errorText().toStdString() << std::endl;
        }
    }

    if( go_on && parser.isSet("help") )
    {
        std::cout << parser.helpText().toStdString() << std::endl;
        go_on = false;
    }

    if(go_on)
    {
        go_on = (parser.positionalArguments().size() == 1);

        if(go_on == false)
        {
            std::cout << "Incorrect command line!" << std::endl;
        }
    }

    if(go_on)
    {
        mOptionFirst = parser.value("first").toInt(&go_on);

        if(go_on == false)
        {
            std::cout << "Incorrect command line arguments!" << std::endl;
        }
    }

    if(go_on)
    {
        mOptionCount = parser.value("count").toInt(&go_on);

        if(go_on == false)
        {
            std::cout << "Incorrect command line arguments!" << std::endl;
        }
    }

    if(go_on)
    {
        mOptionName = parser.value("recname").toStdString();

        go_on = (mOptionName.empty() == false);

        if(go_on == false)
        {
            std::cout << "Incorrect command line arguments!" << std::endl;
        }
    }

    if(go_on)
    {
        mProject.reset(new SLAMProject());
        const std::string project_path = parser.positionalArguments().front().toStdString();
        go_on = mProject->load(project_path.c_str());

        if(go_on == false)
        {
            std::cout << "Could not load project!" << std::endl;
        }
    }

    if(go_on)
    {
        mReconstruction.reset(new Reconstruction());
        mReconstruction->left_camera_to_rig = mProject->getStereoRigCalibration()->left_camera_to_rig;
        mReconstruction->right_camera_to_rig = mProject->getStereoRigCalibration()->right_camera_to_rig;
    }

    if(go_on)
    {
        mModuleOpticalFlow.reset(new SLAMModuleOpticalFlow(mProject));
        mModuleAlignment.reset(new SLAMModuleAlignment(mProject));

        mModuleFeatures.reset(new SLAMModuleFeatures(mProject));
        mModuleStereoMatcher.reset(new SLAMModuleStereoMatcher(mProject));
        mModuleTriangulation.reset(new SLAMModuleTriangulation(mProject));

        mModuleDenseReconstruction.reset(new SLAMModuleDenseReconstruction(mProject));
    }

    return go_on;
}

void SLAMSystem::run()
{
    Image im;
    VideoSourcePtr video;
    int image_count = 0;
    int frame_count = 0;
    bool go_on = true;

    printWelcomeMessage();

    if( go_on )
    {
        go_on = initialize();
    }

    if( go_on )
    {
        video = mProject->getVideo();
        go_on = bool(video);
    }

    if(go_on)
    {
        go_on = video->open();
    }

    if(go_on)
    {
        video->trigger();
        video->read(im);
    }

    while( (mOptionCount < 0 || frame_count < mOptionCount) && go_on && im.isValid() )
    {
        video->trigger();

        if( image_count >= mOptionFirst )
        {
            FramePtr new_frame(new Frame());

            new_frame->id = image_count;
            new_frame->timestamp = im.getTimestamp();
            new_frame->views[0].image = im.getFrame(0);
            new_frame->views[1].image = im.getFrame(1);

            mReconstruction->frames.push_front(new_frame);

            //std::cout << "===> PROCESSING FRAME " << mCurrentFrame->id << " <===" <<std::endl;
            std::cout << "PROCESSING FRAME " << new_frame->id << std::endl;

            processLastFrame();

            std::cout << std::endl;

            // free the image to reduce memory consumption.
            freeOldImages(2);

            frame_count++;
        }

        image_count++;

        video->read(im);
    }

    if( go_on )
    {
        assignMapPointIds();

        video->close();
        video.reset();

        if(mReconstruction->frames.empty() == false)
        {
            mProject->exportReconstruction(mReconstruction, mOptionName);
        }

        finalize();
    }
}

void SLAMSystem::finalize()
{
    mModuleOpticalFlow.reset();
    mModuleAlignment.reset();

    mModuleFeatures.reset();
    mModuleStereoMatcher.reset();
    mModuleTriangulation.reset();

    mModuleDenseReconstruction.reset();

    mProject.reset();
    mReconstruction.reset();
}

void SLAMSystem::printWelcomeMessage()
{
    const std::string logo = BuildInfo::getAsciiLogo();

    std::cout << logo << std::endl;
    std::cout << "Version: " << BuildInfo::getVersionMajor() << "." << BuildInfo::getVersionMinor() << "." << BuildInfo::getVersionRevision() << std::endl;
    std::cout << std::endl;
    std::cout << "Writen by Victor Martin Lac in 2018" << std::endl;
    std::cout << std::endl;
    std::cout << "Build date: " << BuildInfo::getCompilationDate() << std::endl;
    std::cout << "Compiler: " << BuildInfo::getCompilerName() << std::endl;
    std::cout << std::endl;
}

void SLAMSystem::processLastFrame()
{
    FramePtr last_frame = mReconstruction->frames.front();

    /////////////// TRACKING

    // optical flow.

    {
        std::cout << "   OPTICAL FLOW" << std::endl;

        mModuleOpticalFlow->run(mReconstruction->frames);

        std::cout << "      Number of projections on left view: " << last_frame->views[0].projections.size() << std::endl;
        std::cout << "      Number of projections on right view: " << last_frame->views[1].projections.size() << std::endl;
    }

    // alignment.

    {
        std::cout << "   ALIGNMENT" << std::endl;

        mModuleAlignment->run(mReconstruction->frames);

        std::cout << "      Alignment status: " << ( (last_frame->aligned_wrt_previous_frame) ? "ALIGNED" : "NOT ALIGNED" ) << std::endl;
        std::cout << "      Position: " << last_frame->frame_to_world.translation().transpose() << std::endl;
        std::cout << "      Attitude: " << last_frame->frame_to_world.unit_quaternion().coeffs().transpose() << std::endl;
    }

    /////////////// MAPPING

    // features detection.

    {
        std::cout << "   FEATURES DETECTION" << std::endl;

        mModuleFeatures->run(mReconstruction->frames);

        std::cout << "      Num keypoints on left view: " << last_frame->views[0].keypoints.size() << std::endl;
        std::cout << "      Num keypoints on right view: " << last_frame->views[1].keypoints.size() << std::endl;
    }

    // stereo matching.

    {
        std::cout << "   STEREO MATCHING" << std::endl;

        mModuleStereoMatcher->run(mReconstruction->frames);

        std::cout << "      Number of stereo matches: " << last_frame->stereo_matches.size() << std::endl;
    }

    // triangulation.

    {
        std::cout << "   TRIANGULATION" << std::endl;

        mModuleTriangulation->run(mReconstruction->frames);

        std::cout << "      Number of new mappoints: " << mModuleTriangulation->getNumberOfNewMapPoints() << std::endl;
    }

    /////////////// RECONSTRUCTION

    // dense reconstruction.

    {
        std::cout << "   DENSE RECONSTRUCTION" << std::endl;

        mModuleDenseReconstruction->run(mReconstruction->frames);
    }
}

void SLAMSystem::freeOldImages(int num_to_keep)
{
    FrameList::iterator it = mReconstruction->frames.begin();
    int k = 0;

    while( it != mReconstruction->frames.end() && k <= num_to_keep )
    {
        FramePtr frame = *it;

        if( k == num_to_keep )
        {
            frame->views[0].image = cv::Mat();
            frame->views[1].image = cv::Mat();
        }

        it++;
        k++;
    }
}

void SLAMSystem::assignMapPointIds()
{
    int count = 0;

    FrameList::iterator it = mReconstruction->frames.begin();

    while(it != mReconstruction->frames.end())
    {
        FramePtr f = *it;

        for(View& v : f->views)
        {
            for(Projection& p : v.projections)
            {
                if(p.mappoint && p.mappoint->id < 0)
                {
                    p.mappoint->id = count;
                    count++;
                }
            }
        }

        it++;
    }
}
