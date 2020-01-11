"#version 330 core\n"

"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"layout (location = 2) in vec3 normal;\n"

"out vec2 vTexCoord;\n"
"out vec3 vPosition;\n"
"out vec3 vNormal;\n"

"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"

"void main(){\n"
"  vTexCoord = texCoord;\n"
"  vPosition = vec3(model * vec4(position, 1.0));\n"
"  vNormal = mat3(transpose(inverse(model))) * normal;\n"
"  gl_Position = projection * view * vec4(vPosition, 1.0);\n"
"}"
