"#version 330 core\n"

"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"layout (location = 2) in vec3 normal;\n"
// "layout (location = 3) in vec3 tangent;\n"
// "layout (location = 4) in vec3 bitangent;\n"

"out vec2 vTexCoord;\n"
"out vec3 vPosition;\n"
"out vec3 vNormal;\n"
// "out mat3 vTBN;\n"

"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"

"void main(){\n"
"  vTexCoord = texCoord;\n"
"  vPosition = vec3(model * vec4(position, 1.0));\n"
"  vNormal = mat3(transpose(inverse(model))) * normal;\n"

// "  vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));\n"
// "  vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));\n"
// "  vec3 N = normalize(vec3(model * vec4(normal, 0.0)));\n"
// "  vTBN = transpose(mat3(T, B, N));\n"

"  gl_Position = projection * view * vec4(vPosition, 1.0);\n"
"}"
