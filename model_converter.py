import sys, math

def parse_vec3(line):
  return [float(line[1]), float(line[2]), float(line[3])]

def parse_vec2(line):
  return [float(line[1]), float(line[2])]

def norm(vec):
  l = math.sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2])
  if l != 0.0:
    return [vec[0] / l, vec[1] / l, vec[2] / l]
  return [1.0, 0.0, 0.0]

def run():
  if not len(sys.argv) == 2:
    print("you must provide a file to convert")
    quit()

  vertices = []
  uvs = []
  normals = []
  faces = []

  file = open(sys.argv[1], "r")
  for line in file:
    if line[0] == "#":
      continue

    line = line.strip().split(" ")
    if line[0] == "v": # vertex
      vertices.append(parse_vec3(line))
    elif line[0] == "vt": # tex coord
      uvs.append(parse_vec2(line))
    elif line[0] == "vn": # normal
      normals.append(parse_vec3(line))
    elif line[0] == "f": # face
      face = line[1:]

      if len(face) != 3:
        print(line)
        raise Exception("not a triangle!")
        continue

      for i in range(0, len(face)):
        face[i] = face[i].split("/")
        for j in range(0, len(face[i])):
          face[i][j] = int(face[i][j]) - 1

      faces.append(face)

  file.close()

  vertex_data = []
  for face in faces:
    for vertex in face:
      vertex_data.append([
        vertices[vertex[0]],
        uvs[vertex[1]],
        normals[vertex[2]],
      ])

  print("vertices: " + str(len(vertices)))
  print("uvs: " + str(len(uvs)))
  print("normals: " + str(len(normals)))
  print("======================================")

  data = ""
  for i, vertex in enumerate(vertex_data):
    line = "{}f,{}f,{}f,".format(vertex[0][0], vertex[0][1], vertex[0][2]) # position
    line += "{}f,{}f,".format(vertex[1][0], vertex[1][1]) # uv
    line += "{}f,{}f,{}f,".format(vertex[2][0], vertex[2][1], vertex[2][2]) # normal

    # if i < len(vertex_data) - 4: # idk why 4 :)
    #   v1 = vertices[i]
    #   v2 = vertices[i + 2]
    #   v3 = vertices[i + 1]

    #   uv1 = uvs[i]
    #   uv2 = uvs[i + 2]
    #   uv3 = uvs[i + 1]

    #   edge1 = [v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]]
    #   edge2 = [v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]]
    #   deltaUV1 = [uv2[0] - uv1[0], uv2[1] - uv1[1]]
    #   deltaUV2 = [uv3[0] - uv1[0], uv3[1] - uv1[1]]

    #   f = 1.0 / (deltaUV1[0] * deltaUV2[1] - deltaUV2[0] * deltaUV1[1])

    #   tangent = [0.0, 0.0, 0.0]
    #   bitangent = [0.0, 0.0, 0.0]

    #   for j in range(0, 2):
    #     tangent[j] = f * (deltaUV2[1] * edge1[j] - deltaUV1[1] * edge2[j])
    #     bitangent[j] = f * (-deltaUV2[1] * edge1[j] - deltaUV1[0] * edge2[j])

    #   tangent = norm(tangent)
    #   bitangent = norm(bitangent)

    #   line += "{}f,{}f,{}f,".format(tangent[0], tangent[1], tangent[2])
    #   line += "{}f,{}f,{}f,".format(bitangent[0], bitangent[1], bitangent[2])
    # else:
    #   line += "{}f,{}f,{}f,".format(1.0, 0.0, 0.0)
    #   line += "{}f,{}f,{}f,".format(0.0, 1.0, 0.0)

    data += line
  print("{(float[]){"+data.strip(",")+"},"+str(len(vertex_data))+"},")

if __name__ == "__main__":
  run()
