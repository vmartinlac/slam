* make sure that a project has been opened before we do any operation!
* saveCamera must update the model by calling CameraCalibraitonModel::refresh. Same for the others (rig, recording, reconstruction).
* add image width/height to camera serialization.
* before rig calibration, make sure that there exist some camera calibration.
* add a toolbar in which to indicate current project.
* add CSS stylesheet to improve appearance of QTextEdit.
* save last user input into QSettings and recover it on dialog display.
