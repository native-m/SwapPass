-- premake5.lua

-- [ PREPARATION ] --

act = ""
if _ACTION then
	act = _ACTION
end

if act ~= "vs2015" and
	act ~= "vs2017" and
	act ~= "clean" and 
	act ~= "cleanbin" and
	act ~= "cleanall" and
	act ~= "" then
	print("You can't build SwapPass with this action: " .. act)
	print("Please choose one of these actions: vs2015, vs2017")
	os.exit() -- exit the script
end

newaction {
	trigger = "clean",
	description = "clean all build files",
	execute = function()
		print("Cleaning build files... this may take a while")
		os.rmdir("./build")
		print("Done!")
	end
}

newaction {
	trigger = "cleanbin",
	description = "clean all compiled binary files",
	execute = function()
		print("Cleaning compiled binary files... this may take a while")
		os.rmdir("./bin")
		print("Done!")
	end
}

newaction {
	trigger = "cleanall",
	description = "clean all build & compiled binary files",
	execute = function()
		print("Cleaning build & compiled binary files... this may take a while")
		os.rmdir("./build")
		os.rmdir("./bin")
		print("Done!")
	end
}

function isFileExist(path)
	if type(path) ~= "string" then
		return false
	end

	file = io.open(path, "r")
	if file then
		io.close(file)
		return true
	end

	return false
end

-- automatically detect visual studio version
-- and check if visual c++ is installed
if act == "" then
	print("Automatically detect Visual Studio version...")
	
	if isFileExist(os.getenv("ProgramFiles") .. "/MSBuild/Microsoft.Cpp/v4.0/V140/Microsoft.Cpp.props") then
		_ACTION = "vs2015"
	elseif isFileExist(os.getenv("ProgramFiles") .. "/MSBuild/Microsoft.Cpp/v4.0/V150/Microsoft.Cpp.props") then
		_ACTION = "vs2017"
	elseif isFileExist(os.getenv("ProgramFiles(x86)") .. "/MSBuild/Microsoft.Cpp/v4.0/V140/Microsoft.Cpp.props") then
		_ACTION = "vs2015"
	elseif isFileExist(os.getenv("ProgramFiles(x86)") .. "/MSBuild/Microsoft.Cpp/v4.0/V150/Microsoft.Cpp.props") then
		_ACTION = "vs2017"
	else
		print("Visual Studio not found, exiting")
		os.exit()
	end

	act = _ACTION

	ver = {
		["vs2015"] = "2015",
		["vs2017"] = "2017",
	}

	print("Found Visual Studio " .. ver[act])
	print("Generating project...")
end

compiler = {
	["vs2015"] = "v140",
	["vs2017"] = "v141"
}

-- [ WORKSPACE & PROJECT SETUP ] --

workspace("SwapPass_" .. act)
	location("build/" .. act)
	configurations { "Debug", "Release" }
	platforms { "x86", "x64" }
	flags { "MultiProcessorCompile", "NoPCH" }

project("SwapPass_" .. act)
	kind "SharedLib"
	language "C++"
	targetdir("bin/" .. act .. "/%{cfg.buildcfg}_%{cfg.platform}")
	libdirs { "MinHook/lib" }
	includedirs { "MinHook/include" }

	files {
		"src/**.h",
		"src/**.cpp"
	}

	filter { "configurations:Debug", "platforms:x86" }
		architecture "x32"
		defines { "_DEBUG=1", "_PLATFORM_X64=0" }

	filter { "configurations:Debug", "platforms:x64" }
		architecture "x64"
		defines { "_DEBUG=1", "_PLATFORM_X64=1" }

	filter { "configurations:Release", "platforms:x86" }
		architecture "x32"
		defines { "_PLATFORM_X64=0" }
		
	filter { "configurations:Release", "platforms:x64" }
		architecture "x64"
		defines { "_PLATFORM_X64=1" }

	compilerVersion = compiler[act]
	if compilerVersion == nil then
		compilerVersion = "v140"
	end

	filter "configurations:Debug"
		links { "d3d11", "d3dcompiler", "libMinHook-%{cfg.platform}-" .. compilerVersion .. "-mdd" }
		symbols "On"
		optimize "Debug"

	filter "configurations:Release"
		links { "d3d11", "d3dcompiler", "libMinHook-%{cfg.platform}-" .. compilerVersion .. "-md" }
		symbols "Off"
		optimize "Speed"

	filter {}