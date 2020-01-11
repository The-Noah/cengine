import sys

def parse_vec3(line):
  return [float(line[1]), float(line[2]), float(line[3])]

def parse_vec2(line):
  return [float(line[1]), float(line[2])]

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
  for vertex in vertex_data:
    line = "{}f,{}f,{}f,".format(vertex[0][0], vertex[0][1], vertex[0][2]) # position
    line += "{}f,{}f,".format(vertex[1][0], vertex[1][1]) # uv
    line += "{}f,{}f,{}f,".format(vertex[2][0], vertex[2][1], vertex[2][2]) # normal
    data += line
  print("{(float[]){"+data.strip(",")+"},"+str(len(vertex_data))+"},")

if __name__ == "__main__":
  run()
