from dataclasses import dataclass
import os
 
# Clearing the Screen


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
    track_index: int
    scene_index: int
    name: str
    def __str__(self) -> str:
        return self.name

tracks = []
scenes = []
cells = []

def addTrack():
    tracks.append(Track(f"Track {len(tracks) + 1}"))
    # renderMatrix()

def addScene():
    scenes.append(Scene(f"Scene {len(scenes) + 1}"))

def addCell(x, y):
    cells.append(Cell(x, y, f"Cell {len(cells)}"))
   
def hasCell(x,y):
    cell: Cell
    for cell in cells:
        if cell.track_index == x and cell.scene_index == y:
            return cell
    return False
# print matrix
def renderMatrix():
        row = ""
        for y, scene in enumerate(scenes):
            for x, track in enumerate(tracks):
                cell = hasCell(x, y)
                if cell:
                    row = row + "  |  " + " " + str(cell) + " " 
                else:
                    row = row + "  |  " + "  None  " 
            print(row + f" | {scene} |")
            row = ""
            tracksLine=""
        divider = "   "
        for t in tracks:
            tracksLine = tracksLine + "  |   " + str(t) 
            divider = divider + "_____________"
        tracksLine = tracksLine + " |"
        print(divider)
        print(tracksLine)
def renderColumn(index):
    for i, cell in enumerate(matrix[index]):
        print(cell)

def renderRow(index):
    row = ""
    for column in matrix:
        row = row + "  |  " + str(column[index])
    print(row + " |")

def reorderTrack(currentIndex, newIndex):
    track = tracks.pop(currentIndex)
    tracks.insert(newIndex, track)
    current_cells = []
    new_cells = []
    for i, cell in enumerate(cells):
        if cell.track_index == currentIndex:
            current_cells.append(cells.pop(i))
    for i, cell in enumerate(cells):
        if cell.track_index == newIndex:
            new_cells.append(cells.pop(i))
    for c in current_cells:
        c.track_index = newIndex
        cells.append(c)
    for c in new_cells:
        c.track_index = currentIndex
        cells.append(c)
    


def reorderScene(currentIndex, newIndex):
    scene = scenes.pop(currentIndex)
    scenes.insert(newIndex, scene)
    cell: Cell
    current_cells = []
    new_cells = []
    for i, cell in enumerate(cells):
        if cell.track_index == currentIndex:
            current_cells.append(cells.pop(i))
    for i, cell in enumerate(cells):
        if cell.track_index == newIndex:
            new_cells.append(cells.pop(i))
    for c in current_cells:
        c.track_index = newIndex
        cells.append(c)
    for c in new_cells:
        c.track_index = currentIndex
        cells.append(c)

addTrack()
addTrack()
addTrack()
addScene()
addScene()
addScene()

addCell(0,2)
addCell(1,1)
addCell(2,0)
renderMatrix()

msg = ""
reorderTrack(0, 2)
reorderScene(0, 2)
while msg!="q":
    os.system('clear')
    
    renderMatrix()
    msg = input("Command: ")
    if msg == "t":
        addTrack()
    if msg == "s":
        addScene()






