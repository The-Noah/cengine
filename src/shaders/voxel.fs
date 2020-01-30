"#version 330 core\n"

"in vec3 vPosition;\n"
"in float vBrightness;\n"
"in vec2 vTexCoord;\n"
"in vec3 vNormal;\n"

"uniform vec3 camera_position;\n"
"uniform sampler2D diffuse_texture;\n"
"uniform vec3 light_direction = vec3(1.0, 1.0, 1.0);\n"
"uniform float light_intensity = 0.6;\n"

"float calcLight(vec3 position, vec3 lightDir, vec3 normal){\n"
"  float diffuse = max(dot(normal, lightDir), 0.0) * light_intensity;\n"
  /* specular */
"  vec3 viewDir = normalize(camera_position - position);\n"
"  vec3 reflectDir = reflect(-lightDir, normal);\n"
"  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8) * light_intensity;\n"

  "return diffuse + spec;\n"
"}\n"

"void main(){\n"
"  vec3 color = texture(diffuse_texture, vTexCoord).rgb;\n"
"  vec3 normal = normalize(vNormal);"

"  float sun = calcLight(vPosition, normalize(light_direction), normal);\n"

"  float f = pow(clamp(gl_FragCoord.z / gl_FragCoord.w / 1000, 0, 0.8), 2);\n"

"  gl_FragColor = vec4(mix(color * vec3(0.03 + sun) * vBrightness, vec3(0.53, 0.81, 0.92), f), 1.0);\n"
"}"
