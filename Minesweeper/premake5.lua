project "Minesweeper"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   staticruntime "off"

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
      "src/"
   }

   targetdir ("../bin/" .. outputdir .. "-%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "-%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "MS_PLATFORM_WINDOWS" }

   filter "configurations:Debug"
      defines { "MS_DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "MS_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      kind "WindowedApp"
      defines { "MS_DIST" }
      runtime "Release"
      optimize "Speed"
      symbols "Off"
