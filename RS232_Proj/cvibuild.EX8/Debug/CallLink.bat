@"c:\program files (x86)\national instruments\cvi2020\bin\cvilink.exe" -cmd:link_options.txt -flags:0 -expiry:1764799199 > Link.out 2>&1
@if %errorlevel% neq 0 goto err
@echo Link success
@echo Link complete
@exit 0
:err
@echo Link complete
@echo Link failed
@exit 1
:RaiseError
@exit /b 1
