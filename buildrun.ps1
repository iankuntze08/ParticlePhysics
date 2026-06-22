$dec = Read-Host "Clear CMake cache? (y/N)"
if ($dec -eq "y")
{
    rm build -r -fo
}

cmake . -B build
cmake --build build
.\build\Debug\GLVSCode.exe