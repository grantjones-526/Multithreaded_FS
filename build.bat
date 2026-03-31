C:\msys64\mingw64\bin\g++.exe -std=c++17 -pthread -static -g -IFileControlBlock -IOpenFileTable -ISimulatedStorage -IVolumeControlBlock -IDirectory -IFileSystem main.cpp FileSystem\FileSystem.cpp Directory\Directory.cpp FileControlBlock\FCB.cpp OpenFileTable\swOFT.cpp OpenFileTable\ptOFT.cpp SimulatedStorage\SS.cpp VolumeControlBlock\VCB.cpp -o fs_test.exe >build_log.txt 2>&1
type build_log.txt
if %errorlevel% == 0 (
    echo Build succeeded.
    fs_test.exe
) else (
    echo Build failed.
)
