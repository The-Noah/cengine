workspace "cengine"
  configurations {"Debug", "Release"}

project "cengine"
  language "C"
  targetdir "bin/%{cfg.buildcfg}" 

  files {"src/**.h", "src/**.c", "res/textures/**.c"}

  includedirs "include/"

  disablewarnings {"trigraphs"}

  filter "configurations:Debug"
    defines {"DEBUG"}
    symbols "On"
    kind "ConsoleApp"

  filter "configurations:Release"
    defines {"NDEBUG"}
    optimize "Speed"
    kind "WindowedApp"

  filter {"system:windows"}
    libdirs "lib/"
    links {"glew32s", "glfw3", "gdi32", "opengl32"}
		
  filter {"system:not windows"}
    links {"GLEW", "glfw", "rt", "m", "dl", "GL"}
