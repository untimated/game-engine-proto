# Build Shader
param (
    [string]$Path
)

Write-Host $("-"*100) -ForegroundColor DarkGray
Write-Host "Checking and Compiling Shaders" -ForegroundColor Cyan
Write-Host $("-"*100) -ForegroundColor DarkGray

try {
    $pattern = "$Path/resources/shaders/*.hlsl"
    Get-ChildItem $pattern -ErrorAction Stop | ForEach-Object {
        $name = $_.Name;
        $fullname = $_.FullName;
        $global:LASTEXITCODE = 0;
        if ($name -eq "common.hlsl") {
            Write-Host "SKIP $name, $fullname" -ForegroundColor Gray
            return;
        }
        Write-Host "`nCompiling $name" -ForegroundColor Blue;
        dxc -E VS_MAIN -T vs_5_0 -Zi -Fo -Qembed_debug -Qstrip_debug "$path/resources/shaders/$name";
        if($LASTEXITCODE -ne 0) { throw "Error Shader $_" }
        dxc -E PS_MAIN -T ps_5_0 -Zi -Fo -Qembed_debug -Qstrip_debug "$path/resources/shaders/$name";
        if($LASTEXITCODE -ne 0) { throw "Error Shader $_" }
    }
} catch {
    Write-Host "Error Compiling Shaders" -ForegroundColor Red;
    throw $_
}

