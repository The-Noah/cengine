"#version 330 core\n"

"layout (location = 0) in vec4 coord;\n"
"layout (location = 1) in int brightness;\n"
"layout (location = 2) in vec3 normal;\n"
"layout (location = 3) in vec2 texCoord;\n"

"out vec3 vPosition;\n"
"out float vBrightness;\n"
"out vec2 vTexCoord;\n"
"out vec3 vNormal;\n"

"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"

"void main(){\n"
"  vPosition = vec3(model * vec4(coord.xyz, 1.0));\n"
"  vBrightness = float(brightness) * 0.1 + 0.9;\n"
"  vNormal = mat3(transpose(inverse(model))) * normal;\n"
"  vTexCoord = texCoord;\n"

"  gl_Position = projection * view * vec4(vPosition, 1.0);\n"
"}"
