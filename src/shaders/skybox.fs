"#version 330 core\n"

"in vec3 vCoord;\n"

"out vec4 FragColor;\n"

"uniform vec3 color;\n"

"void main(){\n"
"  vec3 sky_normal = normalize(vCoord.xyz);\n"
"  float gradient = dot(sky_normal, vec3(0.0, 1.0, 0.0) * 0.5 /*+ 0.5*/);\n"
"  gl_FragColor = vec4(vec3(gradient) + color, 1.0);\n"
// "  gl_FragColor = vec4(vec3(gradient) + vec3(0.3, 0.6, 0.8), 1.0);\n"
"}";
