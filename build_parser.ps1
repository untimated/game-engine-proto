# Build Engine Parser for Meta
echo "`nCompiling Engine Parser..."
echo "A Preprocessor for our engine meta programming`n"

$flags = '/std:c++17',
'/I./include/', 
'/Zi',
'/EHsc', 
'/DOS=WIN',
'/Fo"./bin/"',
'/Fe"./bin/"'

CL $flags src/EngineParser.cpp 

echo "`nDone`n"
