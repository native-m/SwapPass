# SwapPass

SwapPass allows you to pass a custom post-processing shader into game screen buffer.
WARNING IT STILL IN DEVELOPMENT AND EXPERIMENTAL, STILL NOT DONE YET!


To use this, use SwapPass with ASI Loader or inject the DLL manually into the game.

## How to build

1. Run this command on your shell ```$ ./premake5```.  
   It will automatically generate a Visual Studio 2015/2017 project.  
   Or alternatively you can also specify your Visual Studio version  
   with this command ```$ ./premake5 vs[version_number]```, where ```version_number``` is the  
   version of your Visual Studio (currently SwapPass only support VS2015 and VS2017).
   The generated project files are available inside ```\build``` folder.
2. Open the project inside your Visual Studio.
3. Build it.

## TODO LIST

1. Load custom shader from file (like ENB or ReShade)
2. Load a user configuration (or preset) file for specific shader
3. Implement a shading techniques, passes, and parameters with JSON
4. Get access into depth buffer

This project uses MinHook, licensed under BSD