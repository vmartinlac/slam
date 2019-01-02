#include <Eigen/Eigen>
#include <opencv2/core/eigen.hpp>
#include <opencv2/imgproc.hpp>
#include <utility>
#include "SLAMModuleStereoMatcher.h"
#include "FinitePriorityQueue.h"

//#define DEBUG_SHOW_EPIPOLAR_LINES
//#define DEBUG_SHOW_MATCHES

SLAMModuleStereoMatcher::SLAMModuleStereoMatcher(SLAMContextPtr con) : SLAMModule(con)
{
}

SLAMModuleStereoMatcher::~SLAMModuleStereoMatcher()
{
}

bool SLAMModuleStereoMatcher::init()
{
    SLAMContextPtr con = context();

    mCheckOctave = con->configuration->stereomatcher_check_octave;

    mCheckSymmetry = con->configuration->stereomatcher_check_symmetry;

    mCheckLowe = con->configuration->stereomatcher_check_lowe;
    mLoweRatio = con->configuration->stereomatcher_lowe_ratio;

    mCheckEpipolar = con->configuration->stereomatcher_check_epipolar;
    mEpipolarThreshold = con->configuration->stereomatcher_epipolar_threshold;

    mCameraCalibration[0] = con->calibration->cameras[0].calibration;
    mCameraCalibration[1] = con->calibration->cameras[1].calibration;
    mStereoRigCalibration = con->calibration;

    mFundamentalMatrices[0] = con->calibration->computeFundamentalMatrix(0, 1);
    mFundamentalMatrices[1] = mFundamentalMatrices[0].transpose();

    return true;
}

int SLAMModuleStereoMatcher::matchKeyPoint(SLAMFramePtr f, int view, int i, bool check_symmetry)
{
    const int other_view = (view + 1) % 2;

#ifdef DEBUG_SHOW_EPIPOLAR_LINES
    {
        cv::Mat image = f->views[view].image.clone();
        cv::Mat other_image = f->views[other_view].image.clone();

        cv::circle(image, f->views[view].keypoints[i].pt, 5, cv::Scalar(0,255,0), -1);

        Eigen::Vector3d pointi;
        pointi <<
            mUndistortedPoints[view][i].x,
            mUndistortedPoints[view][i].y,
            1.0;

        Eigen::Vector3d line = mFundamentalMatrices[view] * pointi;

        const double l = line.head<2>().norm();

        if( l > 1.0e-7 )
        {
            line /= l;

            Eigen::Vector2d normal = line.head<2>();

            Eigen::Vector2d tangent;
            tangent.x() = -normal.y();
            tangent.y() = normal.x();

            const Eigen::Vector2d origin = - line.z() * normal;
            const double delta = 2050.0;
            const Eigen::Vector2d A = origin + delta * tangent;
            const Eigen::Vector2d B = origin - delta * tangent;

            const cv::Point2f cvA(A.x(), A.y());
            const cv::Point2f cvB(B.x(), B.y());

            cv::line(other_image, cvA, cvB, cv::Scalar(0,255,0), 3);
            std::cout << A.transpose() << "     " << B.transpose() << std::endl;

            Debug::stereoimshow( image, other_image );
        }
    }
#endif

    FinitePriorityQueueF<int, double, 2> queue;

    for(int j=0; j<f->views[other_view].keypoints.size(); j++)
    {
        bool ok = true;

        if(ok && mCheckOctave)
        {
            ok = ( f->views[view].keypoints[i].octave == f->views[other_view].keypoints[j].octave );
        }

        if(ok)
        {
            const double distance = cv::norm(
                f->views[view].descriptors.row(i),
                f->views[other_view].descriptors.row(j));
            queue.push( j, -distance);
        }
    }

    int ret = -1;

    if( queue.size() >= 2 && mCheckLowe )
    {
        const int j1 = queue.top();
        const double p1 = queue.top_priority();

        queue.pop();

        const int j2 = queue.top();
        const double p2 = queue.top_priority();

        if( (-p1) < mLoweRatio * (-p2) )
        {
            ret = j1;
        }
    }
    else if( queue.size() > 0 )
    {
        ret = queue.top();
    }

    if( ret >= 0 && mCheckEpipolar )
    {
        const int j = ret;

        ret = -1;

        Eigen::Vector3d pointi;
        pointi <<
            mUndistortedPoints[view][i].x,
            mUndistortedPoints[view][i].y,
            1.0;

        Eigen::Vector3d pointj;
        pointj <<
            mUndistortedPoints[other_view][j].x,
            mUndistortedPoints[other_view][j].y,
            1.0;

        Eigen::Vector3d line = mFundamentalMatrices[view] * pointj;

        const double l = line.head<2>().norm();

        if( l > 1.0e-7 )
        {
            line /= l;

            const double val = pointi.transpose() * line;
            //std::cout << val << std::endl;

            if( std::fabs(val) < mEpipolarThreshold )
            {
                //ok = true;
                ret = j;
            }
        }
    }

    if( ret >= 0 && check_symmetry )
    {
        const int other = matchKeyPoint(f, other_view, ret, false);

        if(other != i)
        {
            ret = -1;
        }
    }

    /*
    if( ret >= 0 )
    {
        const int j = ret;

        Eigen::Vector3d pointi;
        pointi <<
            mUndistortedPoints[view][i].x,
            mUndistortedPoints[view][i].y,
            1.0;

        Eigen::Vector3d pointj;
        pointj <<
            mUndistortedPoints[other_view][j].x,
            mUndistortedPoints[other_view][j].y,
            1.0;

        Eigen::Vector3d line = mFundamentalMatrices[view] * pointj;

        const double l = line.head<2>().norm();

        if( l > 1.0e-7 )
        {
            line /= l;

            const double val = pointi.transpose() * line;
            std::cout << val << std::endl;
        }
    }
    */

    return ret;
}

void SLAMModuleStereoMatcher::operator()()
{
    SLAMReconstructionPtr reconstr = context()->reconstruction;

    if( reconstr->frames.empty() ) throw std::runtime_error("internal error");

    SLAMFramePtr f = reconstr->frames.back();

    // compute undistorted key points.

    for(int k=0; k<2; k++)
    {
        if( f->views[k].keypoints.empty() )
        {
            mUndistortedPoints[k].clear();
        }
        else
        {
            std::vector<cv::Point2f> tmp;

            cv::KeyPoint::convert( f->views[k].keypoints, tmp );

            cv::undistortPoints(
                tmp,
                mUndistortedPoints[k], 
                mCameraCalibration[k]->calibration_matrix,
                mCameraCalibration[k]->distortion_coefficients,
                cv::noArray(),
                mCameraCalibration[k]->calibration_matrix);
        }
    }

    // proceed with matching.

    std::vector< std::pair<int,int> >& matches = f->stereo_matches;
    matches.clear();

    for(int i=0; i<f->views[0].keypoints.size(); i++)
    {
        int j = matchKeyPoint(f, 0, i, mCheckSymmetry);

        if( j >= 0 )
        {
            matches.push_back( std::pair<int,int>(i, j) );
            //f->views[0].tracks[i].stereo_match = j;
            //f->views[1].tracks[j].stereo_match = i;
        }
    }

    mUndistortedPoints[0].clear();
    mUndistortedPoints[1].clear();

#ifdef DEBUG_SHOW_MATCHES
    {
        Debug::stereoimshow(
            f->views[0].image,
            f->views[1].image,
            f->views[0].keypoints,
            f->views[1].keypoints,
            matches);
    }
#endif
}

