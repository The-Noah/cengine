"#version 330 core\n"

"layout (location = 0) in vec4 coord;\n"

"out vec4 vTexCoord;\n"

"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"

"void main(){\n"
"  vTexCoord = coord;\n"
"  gl_Position = projection * view * model * vec4(coord.xyz, 1.0);\n"
"}"
