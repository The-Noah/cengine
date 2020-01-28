"#version 330 core\n"

"in float vBrightness;\n"
"in vec2 vTexCoord;\n"

"uniform sampler2D diffuse_texture;\n"

"void main(){\n"
"  vec3 color = texture(diffuse_texture, vTexCoord).rgb * vBrightness;\n"
"  float f = pow(clamp(gl_FragCoord.z / gl_FragCoord.w / 128, 0, 1), 2);\n"

"  gl_FragColor = vec4(mix(color, vec3(0.53, 0.81, 0.92), f), 1.0);\n"
"}"
