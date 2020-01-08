workspace "cengine"
  configurations {"Debug", "Release"}

project "cengine"
  -- kind "WindowedApp"
  kind "ConsoleApp"
  language "C"
  targetdir "bin/%{cfg.buildcfg}" 

  files {"src/**.h", "src/**.c"}

  includedirs "include/"
  libdirs "lib/"

  links {"glew32s", "glfw3", "gdi32"}

  filter "configurations:Debug"
    defines {"DEBUG"}
    symbols "On"

  filter "configurations:Release"
    defines {"NDEBUG"}
    optimize "Speed"

  filter {"system:windows"}
		links {"opengl32"}
		
	filter {"system:not windows"}
		links {"GL"}
