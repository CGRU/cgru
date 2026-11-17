#Requires -Version 5.1
<#
.SYNOPSIS
  Update this CGRU install from the latest GitHub Windows release zip.

.NOTES
  Stops CGRU/Afanasy processes (and Windows services) that belong to this install,
  remembers what was running, mirrors the release (adds/updates/deletes), then restarts them.

  Local files kept even if missing from the zip:
    update_from_github.cmd / .ps1
    software_setup\locate_*  (Keeper defaults)
    .git\  (if present)
#>
[CmdletBinding()]
param(
    [string]$Repo = $(if ($env:CGRU_GITHUB_REPO) { $env:CGRU_GITHUB_REPO } else { 'alexantoshuk/cgru' }),
    [string]$Dest = $(if ($env:CGRU_UPDATE_DEST) { $env:CGRU_UPDATE_DEST } else { (Get-Location).Path })
)

$ErrorActionPreference = 'Stop'

function Write-Step([string]$Message) {
    Write-Host $Message
}

function Get-DestRoot([string]$Path) {
    $resolved = (Resolve-Path -LiteralPath $Path).Path
    return $resolved.TrimEnd('\')
}

function Test-CgruRoot([string]$Root) {
    return (Test-Path -LiteralPath (Join-Path $Root 'version.txt')) -or
           (Test-Path -LiteralPath (Join-Path $Root 'setup.cmd'))
}

function Get-OwnedProcesses([string]$Root) {
    $rootLower = $Root.ToLowerInvariant()
    $keeperMarker = (Join-Path $Root 'utilities\keeper\keeper.py').ToLowerInvariant()
    $owned = @()

    foreach ($proc in Get-CimInstance Win32_Process) {
        $exe = $proc.ExecutablePath
        $cmd = $proc.CommandLine
        $name = $proc.Name
        if (-not $name) { continue }

        $isUnderRoot = $false
        if ($exe -and $exe.ToLowerInvariant().StartsWith($rootLower)) {
            $isUnderRoot = $true
        }

        $isKeeper = $false
        if ($cmd -and $cmd.ToLowerInvariant().Contains($keeperMarker)) {
            $isKeeper = $true
        }

        if (-not $isUnderRoot -and -not $isKeeper) { continue }
        if ($name -match '^(python|pythonw)\.exe$' -and -not $isKeeper) { continue }

        $owned += [pscustomobject]@{
            ProcessId      = [int]$proc.ProcessId
            Name           = $name
            ExecutablePath = $exe
            CommandLine    = $cmd
            IsKeeper       = $isKeeper
            Kind           = if ($isKeeper) { 'keeper' }
                             elseif ($name -match '^afserver') { 'afserver' }
                             elseif ($name -match '^afrender') { 'afrender' }
                             elseif ($name -match '^afwatch') { 'afwatch' }
                             elseif ($name -match '^afservice') { 'afservice' }
                             else { 'other' }
        }
    }

    return $owned
}

function Get-OwnedServices([string]$Root) {
    $rootLower = $Root.ToLowerInvariant()
    $names = @('afservice_server', 'afservice_render')
    $owned = @()
    foreach ($svcName in $names) {
        $svc = Get-Service -Name $svcName -ErrorAction SilentlyContinue
        if (-not $svc) { continue }

        $imagePath = $null
        try {
            $imagePath = (Get-ItemProperty "HKLM:\SYSTEM\CurrentControlSet\Services\$svcName" -ErrorAction Stop).ImagePath
        } catch {
            continue
        }
        if (-not $imagePath) { continue }
        $exeGuess = ($imagePath -replace '^"', '' -split '"')[0]
        if (-not $exeGuess.ToLowerInvariant().StartsWith($rootLower)) { continue }

        $owned += [pscustomobject]@{
            Name       = $svcName
            Status     = [string]$svc.Status
            ImagePath  = $imagePath
            WasRunning = ($svc.Status -eq 'Running')
        }
    }
    return $owned
}

function Stop-OwnedServices($Services) {
    foreach ($svc in $Services) {
        if (-not $svc.WasRunning) { continue }
        Write-Step ("Stopping service {0} ..." -f $svc.Name)
        try {
            Stop-Service -Name $svc.Name -Force -ErrorAction Stop
            $deadline = (Get-Date).AddSeconds(30)
            do {
                Start-Sleep -Milliseconds 500
                $cur = Get-Service -Name $svc.Name -ErrorAction SilentlyContinue
            } while ($cur -and $cur.Status -ne 'Stopped' -and (Get-Date) -lt $deadline)
        } catch {
            Write-Warning ("Could not stop service {0}: {1}" -f $svc.Name, $_.Exception.Message)
        }
    }
}

function Start-OwnedServices($Services) {
    foreach ($svc in $Services) {
        if (-not $svc.WasRunning) { continue }
        Write-Step ("Starting service {0} ..." -f $svc.Name)
        try {
            Start-Service -Name $svc.Name -ErrorAction Stop
        } catch {
            Write-Warning ("Could not start service {0}: {1}" -f $svc.Name, $_.Exception.Message)
        }
    }
}

function Stop-OwnedProcesses($Processes) {
    if (-not $Processes -or $Processes.Count -eq 0) { return }

    $order = @{
        'keeper'    = 0
        'afwatch'   = 1
        'other'     = 2
        'afrender'  = 3
        'afservice' = 4
        'afserver'  = 5
    }
    $sorted = $Processes | Sort-Object { if ($order.ContainsKey($_.Kind)) { $order[$_.Kind] } else { 9 } }, ProcessId

    foreach ($p in $sorted) {
        Write-Step ("Stopping PID {0} ({1}) ..." -f $p.ProcessId, $p.Name)
        try {
            $proc = Get-Process -Id $p.ProcessId -ErrorAction SilentlyContinue
            if (-not $proc) { continue }
            if ($proc.CloseMainWindow()) {
                if (-not $proc.WaitForExit(8000)) {
                    Stop-Process -Id $p.ProcessId -Force -ErrorAction SilentlyContinue
                }
            } else {
                Stop-Process -Id $p.ProcessId -Force -ErrorAction SilentlyContinue
            }
            Start-Sleep -Milliseconds 300
        } catch {
            Write-Warning ("Could not stop PID {0}: {1}" -f $p.ProcessId, $_.Exception.Message)
        }
    }

    $deadline = (Get-Date).AddSeconds(20)
    do {
        $left = @(Get-OwnedProcesses -Root $script:DestRoot | Where-Object {
            $id = $_.ProcessId
            $Processes | Where-Object { $_.ProcessId -eq $id }
        })
        if ($left.Count -eq 0) { break }
        Start-Sleep -Milliseconds 500
    } while ((Get-Date) -lt $deadline)

    $still = @(Get-OwnedProcesses -Root $script:DestRoot)
    if ($still.Count -gt 0) {
        foreach ($p in $still) {
            Write-Warning ("Still running after stop: PID {0} {1}" -f $p.ProcessId, $p.Name)
            Stop-Process -Id $p.ProcessId -Force -ErrorAction SilentlyContinue
        }
        Start-Sleep -Seconds 1
    }
}

function Start-OwnedProcesses($Processes) {
    if (-not $Processes -or $Processes.Count -eq 0) { return }

    $order = @{
        'afserver'  = 0
        'afservice' = 1
        'afrender'  = 2
        'other'     = 3
        'afwatch'   = 4
        'keeper'    = 5
    }
    $unique = @{}
    foreach ($p in $Processes) {
        $key = if ($p.CommandLine) { $p.CommandLine.Trim() }
               elseif ($p.ExecutablePath) { $p.ExecutablePath }
               else { "$($p.Kind):$($p.Name)" }
        if (-not $unique.ContainsKey($key)) {
            $unique[$key] = $p
        }
    }
    $sorted = $unique.Values | Sort-Object { if ($order.ContainsKey($_.Kind)) { $order[$_.Kind] } else { 9 } }

    foreach ($p in $sorted) {
        if ($p.Kind -eq 'afservice') { continue }

        Write-Step ("Restarting {0} ..." -f $p.Kind)

        if ($p.Kind -eq 'keeper') {
            $startCmd = Join-Path $script:DestRoot 'start.cmd'
            if (Test-Path -LiteralPath $startCmd) {
                Start-Process -FilePath 'cmd.exe' -ArgumentList @('/c', "`"$startCmd`"") -WorkingDirectory $script:DestRoot
                Start-Sleep -Seconds 1
                continue
            }
        }

        if ($p.CommandLine -and $p.CommandLine.Trim().Length -gt 0) {
            Start-Process -FilePath 'cmd.exe' -ArgumentList @('/c', $p.CommandLine) -WorkingDirectory $script:DestRoot
            Start-Sleep -Milliseconds 800
            continue
        }

        if ($p.ExecutablePath -and (Test-Path -LiteralPath $p.ExecutablePath)) {
            Start-Process -FilePath $p.ExecutablePath -WorkingDirectory (Split-Path -Parent $p.ExecutablePath)
            Start-Sleep -Milliseconds 800
            continue
        }

        Write-Warning ("No restart command for {0} (PID was {1})" -f $p.Name, $p.ProcessId)
    }
}

function Update-FromGitHub([string]$RepoName, [string]$Root) {
    $assetRegex = [regex]::new('cgru\..*\.windows-x64\.zip$', 'IgnoreCase')
    $api = "https://api.github.com/repos/$RepoName/releases/latest"
    Write-Step "Fetching $api"
    $rel = Invoke-RestMethod -Uri $api -Headers @{
        'User-Agent' = 'cgru-update'
        'Accept'     = 'application/vnd.github+json'
    }
    Write-Step ("Latest release: {0}" -f $rel.tag_name)

    $asset = $rel.assets | Where-Object { $assetRegex.IsMatch($_.name) } | Select-Object -First 1
    if (-not $asset) {
        throw "No release asset matching $($assetRegex) in $($rel.tag_name)"
    }

    Write-Step ("Downloading {0} ({1} MB)" -f $asset.name, [math]::Round($asset.size / 1MB, 1))
    $work = Join-Path $env:TEMP ("cgru_update_" + [guid]::NewGuid().ToString('N'))
    $zip = Join-Path $work $asset.name
    $extract = Join-Path $work 'extract'
    New-Item -ItemType Directory -Force -Path $work, $extract | Out-Null

    try {
        Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $zip -UseBasicParsing
        Write-Step 'Extracting...'
        Expand-Archive -LiteralPath $zip -DestinationPath $extract -Force

        $payload = $extract
        $kids = @(Get-ChildItem -LiteralPath $extract -Force)
        if ($kids.Count -eq 1 -and $kids[0].PSIsContainer) {
            $payload = $kids[0].FullName
        }

        Write-Step ("Mirroring into {0} (extra local files will be removed) ..." -f $Root)
        & robocopy $payload $Root /MIR /IS /IT /NFL /NDL /NJH /NJS /nc /ns /np `
            /XF update_from_github.cmd update_from_github.ps1 `
            /XF locate_*.cmd locate_*.sh `
            /XD .git | Out-Null
        if ($LASTEXITCODE -ge 8) {
            throw "robocopy failed with code $LASTEXITCODE"
        }

        $verFile = Join-Path $Root 'version.txt'
        if (Test-Path -LiteralPath $verFile) {
            Write-Step ("Done. version.txt = {0}" -f (Get-Content -LiteralPath $verFile -Raw).Trim())
        } else {
            Write-Step 'Done.'
        }
    } finally {
        if (Test-Path -LiteralPath $work) {
            Remove-Item -LiteralPath $work -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}

$script:DestRoot = Get-DestRoot -Path $Dest
if (-not (Test-CgruRoot -Root $script:DestRoot)) {
    throw "'$script:DestRoot' does not look like a CGRU root (no version.txt / setup.cmd)."
}

Write-Step "Updating CGRU in: $script:DestRoot"
Write-Step "GitHub repo:      $Repo"
Write-Host ''

$runningProcs = @(Get-OwnedProcesses -Root $script:DestRoot)
$runningServices = @(Get-OwnedServices -Root $script:DestRoot)

if ($runningProcs.Count -gt 0) {
    Write-Step ("Found {0} running process(es) from this install:" -f $runningProcs.Count)
    foreach ($p in $runningProcs) {
        Write-Host ("  - PID {0}  {1}  [{2}]" -f $p.ProcessId, $p.Name, $p.Kind)
    }
} else {
    Write-Step 'No running CGRU/Afanasy processes from this install.'
}

if ($runningServices.Count -gt 0) {
    Write-Step ("Found {0} Windows service(s) from this install:" -f $runningServices.Count)
    foreach ($s in $runningServices) {
        Write-Host ("  - {0}  status={1}  running={2}" -f $s.Name, $s.Status, $s.WasRunning)
    }
}

Stop-OwnedServices -Services $runningServices
Stop-OwnedProcesses -Processes $runningProcs

try {
    Update-FromGitHub -RepoName $Repo -Root $script:DestRoot
} catch {
    Write-Error $_
    Write-Warning 'Update failed — still attempting to restore previously running processes/services.'
    Start-OwnedServices -Services $runningServices
    Start-OwnedProcesses -Processes $runningProcs
    exit 1
}

Start-OwnedServices -Services $runningServices
Start-OwnedProcesses -Processes $runningProcs

Write-Host ''
Write-Step 'Update finished.'
exit 0
