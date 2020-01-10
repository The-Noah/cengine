"#version 330 core\n"

"out vec4 FragColor;\n"

"in float vBrightness;\n"
"in vec2 vTexCoord;\n"
"in vec3 vPosition;\n"
"in vec3 vNormal;\n"

"struct Material{\n"
"  float ambient;\n"
"  float diffuse;\n"
"  float specular;\n"
"  float shininess;\n"
"};\n"

"uniform vec3 cameraPosition;\n"
"uniform Material material;\n"
"uniform sampler2D texture1;\n"

"void main(){\n"
"  vec3 normal = normalize(vNormal);\n"
"  vec3 lightDir = normalize(vec3(0.5, 5.0, -0.2) - vPosition);\n"
  /* diffuse */
"  vec3 diffuse = max(dot(normal, lightDir), 0.0) * vec3(material.diffuse);\n"
  /* specular */
"  vec3 viewDir = normalize(cameraPosition - vPosition);\n"
"  vec3 reflectDir = reflect(-lightDir, normal);\n"
"  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
"  vec3 specular = material.specular * spec * vec3(1.0);\n"

"  FragColor = texture(texture1, vTexCoord) * vec4((vec3(material.ambient) + diffuse + specular) * vec3(vBrightness), 1.0);\n"
"}"
