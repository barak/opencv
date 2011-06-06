@echo off
cd ..
move bin\optcv*.dll . 2> nul
move bin\ippcv*.dll . 2> nul
del /s /f /q *.ncb *.o *.plg *.opt *.aps *.sum *.lst *.obj *.tds *.ilk *.exp log *.dep vc.err vc.log bcc.err bcc.log gcc.err gcc.log icl.err icl.log 2> nul
del /s /f /q bin\*.* 2> nul
for /d /r %%1 in (bin\*.*) do rmdir %%1
del /s /f /q lib\*.* 2> nul
for /d /r %%1 in (lib\*.*) do rmdir %%1
del /s /f /q _temp\*.* 2>nul
for /d /r %%1 in (_temp\*.*) do rmdir %%1
del /s /f /q samples\c\*.exe 2>nul
move optcv*.dll bin 2> nul
move ippcv*.dll bin 2> nul

 