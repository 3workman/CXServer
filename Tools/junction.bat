if "%1" == "" goto :NOVARS
cd %1

:NOVARS
junction.exe ../ThirdParty ../../../Git/CXServer/ThirdParty
junction.exe ../../GoServer/bin/csv ../data/csv
junction.exe ../../GameClient/csv ../data/csv

rem Call ../proto_file/to_cs.bat
rem Call ../proto_file/to_cpp.bat
rem junction.exe ../../GoServer/src/flat ../proto_file/output_go/flat
junction.exe ../../GameClient/Assets/flat ../proto_file/output_cs

cd ../../GoServer/bin
Call generate.exe
cd ../../CXServer/Tools
junction.exe ../../GameClient/Assets/generate ../../GoServer/src/generate_out/rpc/enum/cs
junction.exe ../src/common/enum ../../GoServer/src/generate_out/rpc/enum/cpp


pause