# EmuFlashFS
A crude attempt to replicate the EmuFlash/EmuEMMC functionality from the Nintendo community on Xbox One/Series.


### How does it work?
EmuFlashFS uses a version of MS Detours linked against kernelbase to detect, hook and redirect all calls to certkeys at Xvuc\FlashFs\certkeys.bin, by both CreateFileA and CreateFileW.

### How can I use this?
Create a folder called EmuFlash at D:\ (Dev Scratch). Place a certificate of your choice under this folder as certkeys.bin. 
Inject EmuFlashFS.dll into the process of your choice using a compatible method.
Profit?

### Additional Info
Logging is provided to D:\emuflashlog.txt, refer to this file when encountering errors.
