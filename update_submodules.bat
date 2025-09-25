@echo off
setlocal

echo Parsing .gitmodules file and updating submodules...

:: Check if .gitmodules file exists
if not exist .gitmodules (
    echo .gitmodules file not found!
    echo Please make sure you are running this script from the root of your git repository.
    goto :end
)

:: Iterate through each submodule and update it
for /f "tokens=2" %%p in ('git submodule status --recursive') do (
    echo.
    echo -----------------------------------------------------------------
    echo Processing submodule: %%p
    
    :: Use git submodule update directly on the submodule path
    git submodule update --init --recursive "%%p"

)

echo.
echo -----------------------------------------------------------------
echo All submodules have been processed.
echo -----------------------------------------------------------------

:end
pause
