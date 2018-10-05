#include <QApplication>
#include <QMessageBox>
#include "VimbaCamera.h"
#include "RecordingMainWindow.h"

int main(int num_args, char** args)
{
    QApplication app(num_args, args);

    VimbaCameraManager& vimba = VimbaCameraManager::instance();

    if( vimba.initialize() == false )
    {
        QMessageBox::critical(nullptr, "Error", "Could not initialize Vimba!");
    }
    else if( vimba.getNumCameras() == 0 )
    {
        QMessageBox::critical(nullptr, "Error", "No Allied Vision Technologies camera were detected!");

        vimba.finalize();
    }
    else
    {
        RecordingMainWindow* win = new RecordingMainWindow();

        win->show();

        app.exec();

        delete win;

        vimba.finalize();
    }

    return 0;
}
