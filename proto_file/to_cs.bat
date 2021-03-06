@echo off
set FilePath="./file"
set OutPath="./output_cs"

for  %%i in (%FilePath%/*.fb) do (
   @echo Generate: %%i
   flatc -n --gen-onefile -o %OutPath% %FilePath%/%%i
)

for /d %%i in (%FilePath%/*) do (
   call:SubDirFun %%i
)
pause

:SubDirFun
@echo Sub File Path: %1
set CurSubDirPath=%1
for  %%i in (%FilePath%/%CurSubDirPath%/*.fb) do (
   @echo Generate: %%i
   flatc -n --gen-onefile -o %OutPath%/%CurSubDirPath% %FilePath%/%CurSubDirPath%/%%i
)
