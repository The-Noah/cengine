"#version 330 core\n"

"in float vBrightness;\n"
"in vec2 vTexCoord;\n"

"uniform sampler2D diffuse_texture;\n"

"void main(){\n"
"  gl_FragColor = vec4(texture(diffuse_texture, vTexCoord).rgb * vBrightness, 1.0);\n"
"}"
