#pragma once

#include "MVPnP.h"

namespace MVPnP
{

    class SolverMonoOpenCV : public Solver
    {
    public:

        SolverMonoOpenCV();

        virtual ~SolverMonoOpenCV();

        bool run( const std::vector<View>& views, Sophus::SE3d& rig_to_world, bool use_ransac, std::vector< std::vector<bool> >& inliers) override;
    };
}

