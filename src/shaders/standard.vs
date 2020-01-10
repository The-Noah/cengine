"#version 330 core\n"

"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in float brightness;\n"
"layout (location = 2) in vec3 normal;\n"

"out float vBrightness;\n"
"out vec2 vTexCoord;\n"
"out vec3 vPosition;\n"
"out vec3 vNormal;\n"

"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"

"void main(){\n"
"  vPosition = vec3(model * vec4(position, 1.0));\n"
"  vNormal = mat3(transpose(inverse(model))) * normal;\n"
"  gl_Position = projection * view * vec4(vPosition, 1.0);\n"
"  vBrightness = brightness;\n"

"  if(brightness < 1.02){\n"
"    vTexCoord = position.zy;\n"
"  }else if(brightness < 1.12){\n"
"    vTexCoord = position.xy;\n"
"  }else{\n"
"    vTexCoord = position.xz;\n"
"  }\n"
"}"
