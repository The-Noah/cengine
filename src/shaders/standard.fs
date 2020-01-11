"#version 330 core\n"

"out vec4 FragColor;\n"

"in vec2 vTexCoord;\n"
"in vec3 vPosition;\n"
"in vec3 vNormal;\n"

"struct Material{\n"
"  sampler2D diffuse_texture;\n"
"  sampler2D specular_texture;\n"
"  sampler2D normal_texture;\n"
"  float ambient;\n"
"  float diffuse;\n"
"  float specular;\n"
"  float shininess;\n"
"};\n"

"uniform vec3 cameraPosition;\n"
"uniform Material material;\n"

"void main(){\n"
"  vec3 tex = vec3(texture(material.diffuse_texture, vTexCoord));\n"
"  vec3 ambient = vec3(material.ambient) * tex;\n"
// "  vec3 normal = normalize(vNormal);\n"
"  vec3 normal = normalize(texture(material.normal_texture, vTexCoord).rgb * 2.0 - 1.0);\n"
"  vec3 lightDir = normalize(vec3(1.0, 10.0, -0.5) - vPosition);\n"
  /* diffuse */
"  vec3 diffuse = max(dot(normal, lightDir), 0.0) * vec3(material.diffuse) * tex;\n"
  /* specular */
"  vec3 viewDir = normalize(cameraPosition - vPosition);\n"
"  vec3 reflectDir = reflect(-lightDir, normal);\n"
"  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
"  vec3 specular = material.specular * spec * vec3(texture(material.specular_texture, vTexCoord));\n"

"  FragColor = vec4(ambient + diffuse + specular, 1.0);\n"
"}"
