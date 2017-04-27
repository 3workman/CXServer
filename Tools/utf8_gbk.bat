
for /r ../src %%i in (*.h *.cpp) do (
    
    iconv.exe -f UTF-8 -t GBK %%i > %%i.bkp

    if exist %%i.bkp (
    del %%i
    ren %%i.bkp %%~nxi
    )
)
pause