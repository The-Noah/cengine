"#version 330 core\n"

"in vec3 vCoord;\n"

"out vec4 FragColor;\n"

"void main(){\n"
"  gl_FragColor = vec4(mix(vec3(0.0, 0.1, 0.2), vec3(0.3, 0.6, 0.8), vCoord.y / 50.0 + 0.5), 1.0);\n"
"}";
