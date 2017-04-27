
for /r ../src %%i in (*.h *.cpp) do (
    
    iconv.exe -f GBK -t UTF-8 %%i > %%i.bkp
    
    if exist %%i.bkp (
    del %%i
    ren %%i.bkp %%~nxi
    )
)
pause