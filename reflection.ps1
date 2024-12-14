# Reflecting user exposable structs 
param (
    [string]$Path
)

Write-Host $("-"*100) -ForegroundColor DarkGray
Write-Host "Parsing Headers" -ForegroundColor Cyan
Write-Host $("-"*100) -ForegroundColor DarkGray

try {
    $pattern = "$Path/src/*.h"
    Get-ChildItem $pattern -ErrorAction Stop | ForEach-Object {
        $name = $_.Name;
        $fullname = $_.FullName;
        if ($name -eq "GameGlobals.h") {
            Write-Host "SKIP $name, $fullname" -ForegroundColor Gray
            return
        }
        Write-Host "`nParsing $name" -ForegroundColor Blue;
        ./bin/EngineParser.exe "$path/src/$name" "$path/src/meta/"
    }
} catch {
    Write-Host "Error Meta Parsing" -ForegroundColor Red;
    throw $_
}

