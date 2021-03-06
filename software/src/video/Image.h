#pragma once

#include <array>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <opencv2/core.hpp>

class Image
{
public:

    Image();

    void setInvalid();

    void setValid(double timestamp, const cv::Mat& frame);

    void setValid(double timestamp, const cv::Mat& left_frame, const cv::Mat& right_frame);

    void setValid(double timestamp, const std::vector<cv::Mat>& frames);

    bool isValid();

    int getNumberOfFrames();

    double getTimestamp();

    cv::Mat& getFrame(int idx=0);

    void concatenate( Image& to );

    static void merge( std::vector<Image>& from, Image& to );

protected:

    bool mValid;
    double mTimestamp;
    int mNumberOfFrames;
    std::array<cv::Mat, 2> mFrames;
};

