#include <QScrollArea>
#include <QMessageBox>
#include <QToolBar>
#include <QVBoxLayout>
#include <iostream>
#include "RecordingPlayerDialog.h"

RecordingPlayerDialog::RecordingPlayerDialog(RecordingHeaderPtr header, QWidget* parent)
{
    mCurrentFrame = -1;

    mHeader = header;
    mReader.reset(new RecordingReader(header));
    const bool open_ret = mReader->open();

    if(open_ret == false)
    {
        QMetaObject::invokeMethod(this, "showErrorMessage", Qt::QueuedConnection);
    }

    mTimer = new QTimer(this);

    connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    QToolBar* tb = new QToolBar();
    tb->setToolButtonStyle(Qt::ToolButtonIconOnly);

    QAction* aClose = tb->addAction("Close");
    tb->addSeparator();
    QAction* aFirst = tb->addAction("First frame");
    QAction* aPrevious = tb->addAction("Previous frame");
    QAction* aPlay = tb->addAction("Play");
    QAction* aNext = tb->addAction("Next frame");
    QAction* aLast = tb->addAction("Last frame");

    connect(aClose, SIGNAL(triggered()), this, SLOT(accept()));
    connect(aFirst, SIGNAL(triggered()), this, SLOT(onFirst()));
    connect(aPrevious, SIGNAL(triggered()), this, SLOT(onPrevious()));
    connect(aPlay, SIGNAL(toggled(bool)), this, SLOT(onPlay(bool)));
    connect(aNext, SIGNAL(triggered()), this, SLOT(onNext()));
    connect(aLast, SIGNAL(triggered()), this, SLOT(onLast()));

    aClose->setIcon(QIcon::fromTheme("window-close"));
    aFirst->setIcon(QIcon::fromTheme("media-skip-backward"));
    aPrevious->setIcon(QIcon::fromTheme("media-seek-backward"));
    aPlay->setIcon(QIcon::fromTheme("media-playback-start"));
    aNext->setIcon(QIcon::fromTheme("media-seek-forward"));
    aLast->setIcon(QIcon::fromTheme("media-skip-forward"));

    aPlay->setCheckable(true);

    mSlider = new QSlider();
    mSlider->setOrientation(Qt::Horizontal);
    mSlider->setMinimum(0);
    mSlider->setMaximum(mHeader->num_frames()-1);
    mSlider->setSingleStep(1);

    connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

    VideoWidget* video_widget = new VideoWidget();
    mVideo = video_widget->getPort();

    QScrollArea* scroll = new QScrollArea();
    scroll->setAlignment(Qt::AlignCenter);
    scroll->setWidget(video_widget);

    mLabelFrame = new QLabel();
    mLabelTimestamp = new QLabel();

    QStatusBar* sb = new QStatusBar();
    sb->addPermanentWidget(mLabelFrame);
    sb->addPermanentWidget(mLabelTimestamp);

    QVBoxLayout* lay = new QVBoxLayout();
    lay->addWidget(tb);
    lay->addWidget(mSlider);
    lay->addWidget(scroll);
    lay->addWidget(sb);
    
    setLayout(lay);
    setWindowTitle("Play Recording");
    resize(640, 480);

    showFrame(-1);
    QMetaObject::invokeMethod(this, "showFrame", Qt::QueuedConnection, Q_ARG(int,0));
}

RecordingPlayerDialog::~RecordingPlayerDialog()
{
    mReader->close();
}

void RecordingPlayerDialog::showErrorMessage()
{
    QMessageBox::critical(this, "Error", "Error while loading recording!");
}

void RecordingPlayerDialog::onNext()
{
    if( 0 <= mCurrentFrame && mCurrentFrame+1 < mHeader->num_frames() )
    {
        showFrame(mCurrentFrame+1);
    }
    else
    {
        showFrame(0);
    }

    mSlider->setValue(mCurrentFrame);
}

void RecordingPlayerDialog::onPrevious()
{
    if( 0 <= mCurrentFrame-1 && mCurrentFrame < mHeader->num_frames() )
    {
        showFrame(mCurrentFrame-1);
    }
    else
    {
        showFrame(mHeader->num_frames()-1);
    }

    mSlider->setValue(mCurrentFrame);
}

void RecordingPlayerDialog::onPlay(bool value)
{
    if(value)
    {
        mTimer->start(333);
    }
    else
    {
        mTimer->stop();
    }
}

void RecordingPlayerDialog::onSliderValueChanged(int value)
{
    showFrame(value);
}

void RecordingPlayerDialog::onTimeout()
{
    showFrame( (mCurrentFrame+1)%mHeader->num_frames() );
    mSlider->setValue(mCurrentFrame);
}

void RecordingPlayerDialog::onFirst()
{
    showFrame(0);
    mSlider->setValue(mCurrentFrame);
}

void RecordingPlayerDialog::onLast()
{
    showFrame(mHeader->num_frames() - 1);
    mSlider->setValue(mCurrentFrame);
}

void RecordingPlayerDialog::showFrame(int frame)
{
    bool ok = false;

    if( 0 <= frame && frame < mHeader->num_frames() )
    {
        Image image;
        mReader->seek(frame);
        mReader->trigger();
        mReader->read(image);
        mCurrentFrame = frame;

        if(image.isValid())
        {
            Image concat;
            image.concatenate(concat);

            if( concat.isValid() )
            {
                mVideo->beginWrite();
                mVideo->data().image = concat.getFrame();
                mVideo->endWrite();

                ok = true;
            }
        }
    }

    if(ok)
    {
        mLabelFrame->setText("Frame " + QString::number(mCurrentFrame+1) + "/" + QString::number(mHeader->num_frames()));
        mLabelTimestamp->setText("t =  " + QString::number(mHeader->timestamps[mCurrentFrame]) );
    }
    else
    {
        cv::Mat image(320, 200, CV_8UC3);
        image = cv::Vec3b(0,0,0);

        mVideo->beginWrite();
        mVideo->data().image = image;
        mVideo->endWrite();

        mLabelFrame->setText("N/A");
        mLabelTimestamp->setText("N/A");
    }
}

