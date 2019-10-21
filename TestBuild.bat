@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
cl.exe *.cpp /utf-8 ^
    /GS /GL /W3 /Gy /Zc:wchar_t /Zi /Gm- /sdl /Zc:inline /fp:precise ^
    /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:column ^
    /D "_WINDLL" /D "_UNICODE" /D "UNICODE" ^
    /Fd"./build/" /Fo"./build/" /Fp"./build/" ^
    /link /OUT:"./build/MonitorBar.exe" ^
    /MANIFEST ^
    /LTCG:incremental /NXCOMPAT ^
    /PDB:"./build/MonitorBar.pdb" ^
    /DYNAMICBASE "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" ^
    /IMPLIB:"./build/MonitorBar.lib" ^
    /DEBUG:FULL /MACHINE:X64 /OPT:REF ^
    /MANIFESTUAC:"level='asInvoker' uiAccess='false'" ^
    /ManifestFile:"./build/MonitorBar.exe.intermediate.manifest" ^
    /OPT:ICF ^
    /ERRORREPORT:PROMPT ^
    /NOLOGO ^
    /TLBID:1 