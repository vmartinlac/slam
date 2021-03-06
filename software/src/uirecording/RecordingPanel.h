#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QListView>

class Project;

class RecordingPanel : public QWidget
{
    Q_OBJECT

public:

    RecordingPanel(Project* project, QWidget* parent=nullptr);

protected slots:

    void onNewMonoRecording();
    void onNewStereoRecording();
    void onNewArbitraryRecording();
    void onNewRecording(int num_cameras);
    void onPlayRecording();
    void onRenameRecording();
    void onDeleteRecording();

    void onSelect(const QModelIndex&);
    void onModelChanged();

protected:

    Project* mProject;
    QListView* mView;
    QTextEdit* mText;
};

