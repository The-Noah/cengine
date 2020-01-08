workspace "cengine"
  configurations {"Debug", "Release"}

project "cengine"
  kind "WindowedApp"
  language "C"
  targetdir "bin/%{cfg.buildcfg}" 

  files {"src/**.h", "src/**.c"}

  includedirs "include/"
  libdirs "lib/"

  links "glfw3"

  filter "configurations:Debug"
    defines {"DEBUG"}
    symbols "On"

  filter "configurations:Release"
    defines {"NDEBUG"}
    optimize "Speed"

  filter {"system:windows"}
		links {"OpenGL32"}
		
	filter {"system:not windows"}
		links {"GL"}
