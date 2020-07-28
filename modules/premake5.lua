moduleDirNames = os.matchdirs("*")

for _, moduleName in ipairs(moduleDirNames) do
  project(moduleName)
    kind "StaticLib"

    targetdir(moduleName.."/bin")
    objdir(moduleName.."/obj")

    files {moduleName.."/**.h", moduleName.."/**.c"}
end
