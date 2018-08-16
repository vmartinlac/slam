#pragma once

#include <QWidget>
#include <QLabel>
#include "SLAMOutput.h"

class StatsWidget : public QWidget
{
    Q_OBJECT
public:

    StatsWidget(
        SLAMOutput* slam,
        QWidget* parent=nullptr);

protected:

    QLabel* m_num_landmarks;
    SLAMOutput* m_slam;
};
