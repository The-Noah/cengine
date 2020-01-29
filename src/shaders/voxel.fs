"#version 330 core\n"

"in vec3 vPosition;\n"
"in float vBrightness;\n"
"in vec2 vTexCoord;\n"
"in vec3 vNormal;\n"

"uniform vec3 camera_position;\n"
"uniform sampler2D diffuse_texture;\n"

"void main(){\n"
// "  vec3 color = texture(diffuse_texture, vec2(fract(vTexCoord.x + vTexCoord.z) / 4.0, vTexCoord.y)).rgb;\n"
"  vec3 color = texture(diffuse_texture, vTexCoord).rgb;\n"

"  vec3 normal = normalize(vNormal);"
"  vec3 lightDir = normalize(vec3(0.7, 0.8, 0.1));\n"
"  float diffuse = max(dot(normal, lightDir), 0.0) * 0.8;\n"
  /* specular */
"  vec3 viewDir = normalize(camera_position - vPosition);\n"
"  vec3 reflectDir = reflect(-lightDir, normal);\n"
"  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32) * 0.7;\n"

"  float f = pow(clamp(gl_FragCoord.z / gl_FragCoord.w / 1000, 0, 0.8), 2);\n"

"  gl_FragColor = vec4(mix(color * vec3(0.15 + diffuse + spec) * vBrightness, vec3(0.53, 0.81, 0.92), f), 1.0);\n"
"}"
