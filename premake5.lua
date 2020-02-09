workspace "cengine"
  configurations {"Debug", "Release"}

  newaction {
    ["trigger"] = "clean",
    ["description"] = "Delete generated project and build files",
    ["onStart"] =
      function()
	      print "Cleaning files..."
      end,
    ["execute"] =
      function()
        if _TARGET_OS == "windows" then
          os.execute("IF EXIST bin ( RMDIR /S /Q bin )")
          os.execute("IF EXIST obj ( RMDIR /S /Q obj )")
          os.execute("del Makefile *.make")
        else
          os.execute("rm -rf bin obj Makefile *.make")
        end
      end,
    ["onEnd"] =
      function()
	      print "Done."
      end
  }

  newaction {
    ["trigger"] = "dev",
    ["description"] = "Build and run",
    ["execute"] =
      function()
        os.execute("premake5 gmake2")
        if _TARGET_OS == "windows" then
          os.execute("mingw32-make && bin\\Debug\\cengine.exe")
        else
          os.execute("make && ./bin/Debug/cengine")
        end
      end
  }

project "cengine"
  language "C"
  targetdir "bin/%{cfg.buildcfg}" 

  files {"src/**.h", "src/**.c"}

  includedirs "include/"
  staticruntime "On"
  flags {"LinkTimeOptimization"}
  links {"pthread"}

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
