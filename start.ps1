# Stock Analyzer Startup Script (Windows)
# Run with: powershell -ExecutionPolicy Bypass -File start.ps1

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$LogDir = Join-Path $ScriptDir "logs"
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

Write-Host "Starting Stock Analyzer Application..."
Write-Host ""

# Kill any existing processes
Write-Host "Stopping existing processes..."
Get-Process -Name "stock_analyzer" -ErrorAction SilentlyContinue | Stop-Process -Force
Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like "*http.server*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like "*proxy-server*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Start-Sleep -Seconds 1

# Start C++ backend on port 8080
Write-Host "Starting C++ backend server on port 8080..."
$backendExe = Join-Path $ScriptDir "backend\build\Release\stock_analyzer.exe"
if (-not (Test-Path $backendExe)) {
    Write-Host "ERROR: Backend executable not found at $backendExe"
    Write-Host "Build it first: cd backend\build && cmake .. && cmake --build . --config Release"
    exit 1
}
$backendWorkDir = Join-Path $ScriptDir "backend\build"
New-Item -ItemType Directory -Force -Path "$backendWorkDir\database" | Out-Null
$backend = Start-Process -FilePath $backendExe `
    -WorkingDirectory $backendWorkDir `
    -RedirectStandardOutput "$LogDir\stock_analyzer.log" `
    -RedirectStandardError  "$LogDir\stock_analyzer_err.log" `
    -PassThru -WindowStyle Hidden
Write-Host "Backend started (PID: $($backend.Id))"
Write-Host ""

# Start CORS proxy on port 8081
Write-Host "Starting CORS proxy on port 8081..."
$proxyScript = Join-Path $ScriptDir "proxy-server.js"
$proxy = Start-Process -FilePath "node" -ArgumentList "`"$proxyScript`"" `
    -RedirectStandardOutput "$LogDir\proxy.log" `
    -RedirectStandardError  "$LogDir\proxy_err.log" `
    -PassThru -WindowStyle Hidden
Write-Host "Proxy started (PID: $($proxy.Id))"
Write-Host ""

# Start frontend HTTP server on port 3000
Write-Host "Starting frontend server on port 3000..."
$frontendDir = Join-Path $ScriptDir "frontend"
$frontend = Start-Process -FilePath "python" -ArgumentList "-m http.server 3000 --directory `"$frontendDir`"" `
    -RedirectStandardOutput "$LogDir\frontend.log" `
    -RedirectStandardError  "$LogDir\frontend_err.log" `
    -PassThru -WindowStyle Hidden
Write-Host "Frontend started (PID: $($frontend.Id))"
Write-Host ""

Start-Sleep -Seconds 2

Write-Host "All servers running!"
Write-Host ""
Write-Host "Access the application:"
Write-Host "   Frontend:  http://localhost:3000"
Write-Host "   API Proxy: http://localhost:8081"
Write-Host "   Backend:   http://localhost:8080"
Write-Host ""
Write-Host "Logs are in: $LogDir"
Write-Host ""
Write-Host "To stop all servers, run: stop.ps1"

# Write a companion stop script
$stopScript = Join-Path $ScriptDir "stop.ps1"
@'
Get-Process -Name "stock_analyzer" -ErrorAction SilentlyContinue | Stop-Process -Force
Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like "*http.server*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like "*proxy-server*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Write-Host "All Stock Analyzer processes stopped."
'@ | Set-Content -Path $stopScript
