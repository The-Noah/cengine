"#version 330 core\n"

"in vec4 vTexCoord;\n"

"void main(){\n"
"  gl_FragColor = vec4(vTexCoord.w / 128.0, vTexCoord.w / 256.0, vTexCoord.w / 512.0, 1.0);\n"
"}"
