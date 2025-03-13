class MidiClip:

    class States:
        STOPPED = "STOPPED"
        LAUNCHING_PLAY = "LAUNCHING_PLAY"
        LAUNCHING_RECORDING = "LAUNCHING_RECORDING"
        RECORDING_INITIAL = "RECORDING_INITIAL"
        RECORDING = "RECORDING"
        PLAYING = "PLAYING"
        STOPPING = "STOPPING"
        DISABLED = "DISABLED"

        
    def __init__(self, name):
        self.name = name
        self.data = {}

    def load_notes(self):
        for note in self.clip.get_notes():
            self.notes.append(note)
