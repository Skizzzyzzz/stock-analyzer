Get-Process -Name "stock_analyzer" -ErrorAction SilentlyContinue | Stop-Process -Force
Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like "*http.server*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like "*proxy-server*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Write-Host "All Stock Analyzer processes stopped."
