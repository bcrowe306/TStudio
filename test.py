from dataclasses import dataclass


matrix=[]
@dataclass
class Scene:
    name: str

    def __str__(self) -> str:
        return self.name

@dataclass
class Track:
    name:str
    def __str__(self) -> str:
        return self.name


@dataclass
class Cell:
    x: int
    y: int

    def __str__(self) -> str:
        return f"{self.x}:{self.y}"


tracks = [
    Track("Drums"),
    Track("Bass"),
]
scenes = [
    Scene("Intro"),
    Scene("Verse")
]


for ix, _ in enumerate(tracks):
    new_scenes = []
    for iy, _ in enumerate(scenes):
        new_scenes.append(Cell(ix, iy))
    matrix.append(new_scenes)

def addTrack():
    new_scenes = []
    for i, y in enumerate(scenes):
        new_scenes.append(Cell(len(matrix), i))
    matrix.append(new_scenes)

def addScene():
    scenes.append(Scene("Hook"))
    for i, sce in enumerate(matrix):
        sce.append(Cell(i, len(sce)))

# print matrix
def renderMatrix():

  row = ""

  for r, _ in enumerate(matrix):

      for i, x in enumerate(matrix):
          for i2, y in enumerate(x):
              if i2 == r:
                  row = row + " " + str(y)
      print(row)
      row = ""
addTrack()
addScene()
addTrack()
renderMatrix()
