"#version 330 core\n"

"layout (location = 0) in vec4 coord;\n"
"layout (location = 1) in int brightness;\n"
"layout (location = 2) in vec3 normal;\n"

"out vec3 vPosition;\n"
"out float vBrightness;\n"
"out vec2 vTexCoord;\n"
"out vec3 vNormal;\n"

"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"

"void main(){\n"
"  vPosition = vec3(model * vec4(coord.xyz, 1.0));\n"
"  vBrightness = float(brightness) * 0.1 + 1.0;\n"
"  vNormal = mat3(transpose(inverse(model))) * normal;\n"

"  if(vBrightness < 1.02){\n"
"    vTexCoord = coord.zy;\n"
"  }else if(vBrightness < 1.12){\n"
"    vTexCoord = coord.xy;\n"
"  }else{\n"
"    vTexCoord = coord.xz;\n"
"  }\n"

"  gl_Position = projection * view * vec4(vPosition, 1.0);\n"
"}"
