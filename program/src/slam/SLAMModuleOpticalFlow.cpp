#include "SLAMModuleOpticalFlow.h"

#include "Debug.h"
#include "SLAMModuleOpticalFlow.h"

#define DEBUG_SHOW_FEATURES

SLAMModuleOpticalFlow::SLAMModuleOpticalFlow(SLAMProjectPtr project) : SLAMModule(project)
{
    const int size = project->getParameterInteger("optical_flow_window_size", 21);

    const int min_width = 150;
    const int max_level = std::floor( std::log(double(project->getLeftCameraCalibration()->image_size.width)/double(min_width)) / std::log(2.0) );

    mLKT = cv::SparsePyrLKOpticalFlow::create();

    mLKT->setWinSize(cv::Size(size, size));
    mLKT->setMaxLevel(max_level);
}

void SLAMModuleOpticalFlow::run(FramePtr frame)
{
    if( frame->previous_frame )
    {
        for(int i=0; i<2; i++)
        {
            processView( frame->previous_frame->views[i], frame->views[i] );
        }
    }
    else
    {
        frame->frame_to_world = Sophus::SE3d();
        frame->aligned_wrt_previous_frame = false;
    }
}

void SLAMModuleOpticalFlow::processView(const View& prev_view, View& curr_view)
{
    const std::vector<Projection>& proj_prev = prev_view.projections;
    std::vector<Projection>& proj_curr = curr_view.projections;

    proj_curr.clear();

    if( proj_prev.empty() == false );
    {
        const int N = proj_prev.size();

        std::vector<cv::Point2f> points_prev(N);

        for(int i=0; i<N; i++)
        {
            points_prev[i] = proj_prev[i].point;
        }

        std::vector<cv::Point2f> points_curr;
        std::vector<uint8_t> status;

        if( N > 0 )
        {
            mLKT->calc(
                prev_view.image,
                curr_view.image,
                points_prev,
                points_curr,
                status);
        }


        for(int i=0; i<N; i++)
        {
            if( status[i] > 0 && proj_prev[i].max_lifetime > 0 )
            {
                Projection proj;
                proj.point = points_curr[i];
                proj.mappoint = proj_prev[i].mappoint;
                proj.max_lifetime = proj_prev[i].max_lifetime - 1;
                proj.type = PROJECTION_TRACKED;

                proj_curr.push_back(proj);
            }
        }
    }
}
