workspace "l65" do
    local workspace_location = "build"

    location(workspace_location)
    startproject "l65"

    configurations { "debug", "release" }
    
    flags { "FatalCompileWarnings", "NoPCH", "StaticRuntime" }

    platforms { "x86", "x64" }
    filter "platforms:x86"
        architecture "x86"
    filter "platforms:x64"
        architecture "x86_64"
    filter {}

    filter "configurations:debug"
        defines { "_DEBUG" }
        flags { "Symbols" }
    filter "configurations:release"
        defines { "NDEBUG" }
        flags { "LinkTimeOptimization", "NoBufferSecurityCheck", "NoFramePointer" }
        optimize "On"
        floatingpoint "Fast"
        vectorextensions "SSE2"
    filter {}

    filter "action:vs*"
        defines { "WIN32", "_CRT_SECURE_NO_WARNINGS", "_CRT_SECURE_NO_DEPRECATE" }
        flags { "MultiProcessorCompile", "NoEditAndContinue", "NoIncrementalLink", "NoManifest", "NoMinimalRebuild" }
        disablewarnings { "4103", "4554", "6255", "6262", "28278" }
        buildoptions { "/volatile:iso" }
    filter {}

    project "lua"
        location(workspace_location)
        kind "StaticLib"
        language "C"

        filter "action:vs*"
            disablewarnings { "4244", "4702" }
        filter {}

        src = "src/lua"
        includedirs { src }
        files { src .. "/**.h", src .. "/**.c" }

    project "l65"
        location(workspace_location)
        kind "ConsoleApp"
        language "C"
        targetdir "build/bin"

        includedirs { "src/lua" }
        files { "src/main.c" }

        links { "lua" }
end
