import rtmidi
import time
import sys
import os
import pathlib
directory = [f for f in pathlib.Path().iterdir()]
def prRed(skk): print("\033[91m{}\033[00m" .format(skk))

class ScrollView:
    def __init__(self, length: list, view_size: int=5, cursor: int=0, scroll_padding: int = 1):
        self.start = 0
        self.view_size = view_size
        self.scroll_padding = scroll_padding if length > scroll_padding * 3 else 0
        self.end = self.view_size 
        self._length = length
        self.view_last_index = self.view_size - 1
        self.cursor = cursor
        self.view_index = self.cursor - self.start

    def limit_bounds(self, number, lower_bound, upper_bound):
        if number < lower_bound:
            return lower_bound
        elif number > upper_bound:
            return upper_bound
        else:
            return number
        
    def get_offset(self, start, end):
        if self.cursor < start:
            return self.cursor - start
        elif self.cursor > end:
            return self.cursor - end
        else:
            return 0
    def render_view(self):
        if self.length >= self.view_size:
            lower_end = self.length-self.view_size
        elif self.view_size > self.length:
            lower_end = 0

        
        lower_bounds = (0, lower_end)
        upper_bounds = (self.view_size if self.view_size < self.length else self.length, self.length)
        # set Bounds of Cursor
        
        self.offset = self.get_offset(self.start + self.scroll_padding, self.end - (self.scroll_padding + 1))
        self.start = self.limit_bounds(self.start + self.offset, lower_bounds[0], lower_bounds[1])
        self.end = self.limit_bounds(self.end + self.offset, upper_bounds[0], upper_bounds[1])
        self.view_index = self.cursor - self.start

    def shift(self, amount)-> tuple[int:int:int]:
        self.cursor = self.cursor + amount
        self.render_view()
    
    def increment(self)-> tuple[int:int:int]:
        self.shift(1)

    def decrement(self)-> tuple[int:int:int]:
        self.shift(-1)

    @property
    def cursor(self) -> int: 
        return self._cursor

    @cursor.setter
    def cursor(self, value: int) -> None:
        self._cursor = self.limit_bounds(value, 0, self.length-1)
        self.render_view()

    @property
    def length(self) -> int: 
        return self._length

    @length.setter
    def length(self, value: int) -> None:
        self._length = value
        self.cursor = 0
        self.render_view()


class MidiInputHandler(object):
    def __init__(self, port, directory):
        self.scrollView = ScrollView(len(directory), 5)
        self.port = port
        self.directory = directory

    def __call__(self, event, data=None):
        message, deltatime = event
        os.system("clear")
        print(message)
        status, data1, data2 = message
        if status == 176 and data1 == 14 and data2 == 1:
           self.scrollView.increment()
        if status == 176 and data1 == 14 and data2 == 127:
           self.scrollView.decrement()
        if status == 144 and data1 >= 36:
           self.scrollView.cursor = data1 - 36
        if status == 176 and data1 == 48 and data2 == 127:
           pathItem: pathlib.Path = self.directory[self.scrollView.cursor]
           if pathItem.is_dir():
               self.directory = [f for f in pathItem.iterdir()]
               self.scrollView.length = len(self.directory)
        if status == 176 and data1 == 49 and data2 == 127:
            pathItem: pathlib.Path = self.directory[self.scrollView.cursor]
            self.directory = [f for f in pathItem.parent.parent.iterdir()]
            self.scrollView.length = len(self.directory)
            


        # print(f"start: {self.scrollView.start}, end: {self.scrollView.end}, cursor: {self.scrollView.cursor}, view_index: {self.scrollView.view_index}, offset: {self.scrollView.offset}")
        print(self.directory[self.scrollView.cursor])
        for i in range(self.scrollView.start, self.scrollView.end):
            if i == self.scrollView.cursor:
                prRed(f"{self.directory[i]}")
            else:
                print(f"{self.directory[i]}")
            

from rtmidi.midiutil import open_midiinput


port = "Ableton Push 2 Live Port"

try:
    midiin, port_name = open_midiinput(port)
except (EOFError, KeyboardInterrupt):
    sys.exit()

print("Attaching MIDI input callback handler.")
midiin.set_callback(MidiInputHandler(port_name, directory))

print("Entering main loop. Press Control-C to exit.")
try:
    # Just wait for keyboard interrupt,
    # everything else is handled via the input callback.
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print('')
finally:
    print("Exit.")
    midiin.close_port()
    del midiin



