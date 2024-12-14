# Compile Everything
param (
    [string]$ProjectPath,
    [switch]$ParseMeta,
    [switch]$CheckShader
)

# $project_path = "C:/Users/psmmicha0040/Documents/Project/Prototypes/GameProject";
$project_path = $ProjectPath;
$project_name = "Game.exe";
$d_base_path = "/D`"PROJECT_PATH=$project_path`"";
$f_exec = "/Fe`"$project_path/$project_name`"";
$f_obj = "/Fo`"$project_path/build/`"";
$f_map = "/Fm`"$project_path/build/`"";

$flags = '/std:c++17',
'/I./include/', 
'/I./include/utils/freetype/', 
'/I./src',
'/MP',
'/MDd',
'/Zi',
'/EHsc', 
$f_exec,
$f_obj, 
$d_base_path,
'/D"DEBUG=1"',
'/D"OS=WIN"'
;
$source = 
'src/EnginePlatform.cpp',
'src/EngineCore.cpp',
'src/utils/Debug.cpp',
'src/utils/RUID.cpp',
'src/utils/WICTextureLoader.cpp',
'src/core/SceneGraph.cpp',
'src/core/GameObject.cpp',
'src/core/GameResource.cpp',
'src/core/GameLoader.cpp',
'src/core/Math.cpp',
'src/core/Geometry.cpp',
'src/core/DebugDraw.cpp',
'src/core/Physics.cpp',
'src/platform/Graphics_d3d.cpp',
'src/platform/Input_win.cpp',
'src/platform/IO_win.cpp',
'src/platform/FontLoader_win.cpp',
'src/api/Engine.cpp';

$gamesource = "$project_path/src/*.cpp";

$lib = 
'user32.lib',
'D3D11.lib', 
'DXGI.lib', 
'D3DCompiler.lib',
'DXGUID.lib',
'./lib/freetype.lib',
'Ole32.lib';

if($ParseMeta) {
    try {
        ./reflection -Path $project_path
        Write-Host "OK" -ForegroundColor Green
    } catch {
        Write-Host "Compile Engine Error, $_ `n" -ForegroundColor Red
        Write-Host "Exit program `n" -ForegroundColor Red
        exit 1;
    }
}

if($CheckShader) {
    try {
        ./build_shader -Path $project_path
        Write-Host "OK" -ForegroundColor Green
    } catch {
        Write-Host "Compile Engine Error, $_ `n" -ForegroundColor Red
        Write-Host "Exit program `n" -ForegroundColor Red
        exit 1;
    }
}

Write-Host $("-"*100) -ForegroundColor DarkGray
Write-Host "Compiling Engine" -ForegroundColor Cyan
Write-Host $("-"*100) -ForegroundColor DarkGray

Write-Host $flags;
CL $flags $source $gamesource /link /INCREMENTAL $lib

Write-Host "OK" -ForegroundColor Green

