
add_library(
    salma_calibration
    SHARED
    CameraCalibration.h
    CameraCalibration.cpp
    StereoRigCalibration.h
    StereoRigCalibration.cpp
)

target_include_directories(salma_calibration INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(salma_calibration PUBLIC Qt5::Core Eigen3::Eigen Sophus::Sophus opencv_core)

install(TARGETS salma_calibration LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

