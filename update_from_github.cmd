@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem Update this CGRU install from the latest GitHub release asset for this OS.
rem Run from the CGRU root (the folder that contains version.txt / setup.cmd).
rem
rem Optional env:
rem   CGRU_GITHUB_REPO  owner/name  (default: alexantoshuk/cgru)
rem   CGRU_UPDATE_DEST   target dir  (default: current directory)

set "REPO=%CGRU_GITHUB_REPO%"
if "%REPO%"=="" set "REPO=alexantoshuk/cgru"

set "DEST=%CGRU_UPDATE_DEST%"
if "%DEST%"=="" set "DEST=%CD%"

if not exist "%DEST%\version.txt" if not exist "%DEST%\setup.cmd" (
    echo ERROR: "%DEST%" does not look like a CGRU root ^(no version.txt / setup.cmd^).
    echo cd into the unpacked release folder and run again.
    exit /b 1
)

rem Pick asset name pattern for this OS
set "ASSET_REGEX=cgru\..*\.windows-x64\.zip$"
echo Updating CGRU in: %DEST%
echo GitHub repo:      %REPO%
echo Asset pattern:    %ASSET_REGEX%
echo.

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop';" ^
  "$repo='%REPO%'; $dest=(Resolve-Path -LiteralPath '%DEST%').Path;" ^
  "$re=[regex]::new('%ASSET_REGEX%', 'IgnoreCase');" ^
  "$api='https://api.github.com/repos/' + $repo + '/releases/latest';" ^
  "Write-Host ('Fetching ' + $api);" ^
  "$rel=Invoke-RestMethod -Uri $api -Headers @{ 'User-Agent'='cgru-update'; 'Accept'='application/vnd.github+json' };" ^
  "Write-Host ('Latest release: ' + $rel.tag_name);" ^
  "$asset=$rel.assets | Where-Object { $re.IsMatch($_.name) } | Select-Object -First 1;" ^
  "if (-not $asset) { throw ('No release asset matching ' + $re + ' in ' + $rel.tag_name) };" ^
  "Write-Host ('Downloading ' + $asset.name + ' (' + [math]::Round($asset.size/1MB,1) + ' MB)');" ^
  "$work=Join-Path $env:TEMP ('cgru_update_' + [guid]::NewGuid().ToString('N'));" ^
  "$zip=Join-Path $work $asset.name;" ^
  "$extract=Join-Path $work 'extract';" ^
  "New-Item -ItemType Directory -Force -Path $work,$extract | Out-Null;" ^
  "try {" ^
  "  Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $zip -UseBasicParsing;" ^
  "  Write-Host 'Extracting...';" ^
  "  Expand-Archive -LiteralPath $zip -DestinationPath $extract -Force;" ^
  "  $root=$extract;" ^
  "  $kids=@(Get-ChildItem -LiteralPath $extract -Force);" ^
  "  if ($kids.Count -eq 1 -and $kids[0].PSIsContainer) { $root=$kids[0].FullName };" ^
  "  Write-Host ('Applying into ' + $dest + ' ...');" ^
  "  robocopy $root $dest /E /IS /IT /NFL /NDL /NJH /NJS /nc /ns /np | Out-Null;" ^
  "  $rc=$LASTEXITCODE;" ^
  "  if ($rc -ge 8) { throw ('robocopy failed with code ' + $rc) };" ^
  "  $verFile=Join-Path $dest 'version.txt';" ^
  "  if (Test-Path -LiteralPath $verFile) { Write-Host ('Done. version.txt = ' + (Get-Content -LiteralPath $verFile -Raw).Trim()) }" ^
  "  else { Write-Host 'Done.' }" ^
  "} finally {" ^
  "  if (Test-Path -LiteralPath $work) { Remove-Item -LiteralPath $work -Recurse -Force -ErrorAction SilentlyContinue }" ^
  "}"

if errorlevel 1 (
    echo.
    echo Update failed.
    exit /b 1
)

echo.
echo Restart Keeper / Afanasy tools if they were running.
exit /b 0
