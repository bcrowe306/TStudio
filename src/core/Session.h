#ifndef SESSION_H
#define SESSION_H

#include "LabSound/LabSound.h"
#include "core/TrackNode.h"
#include "core/TrackNode.h"
#include "core/MidiClip.h"
#include "core/Playhead.h"
#include "library/UUID_Gen.h"
#include "library/Parameter.h"
#include "library/EventRegistry.h"
#include <utility>
#include <vector>
#include <variant>
#include <optional>
#include <string>
#include <memory>
#include <any>

using namespace lab;
using std::vector;
using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::any;
using std::variant;
using std::optional;


// TODO: Implement Undo/Redo - Command Pattern

namespace tstudio {

using MidiClipType = shared_ptr<MidiClip>;

    struct Scene {
        Scene() :id(GenerateUUID()) {}
        uuid id;
        StringParam name = StringParam("name", "Name", "", "Text");
        FloatParam tempo = FloatParam("tempo", "Tempo", 120.f, "Text");
        std::pair<int, int> time_sig{4,4};
    };

    class Session : public EventBase {
    public:
        // Constructor
        Session(shared_ptr<AudioContext>, shared_ptr<Playhead>);

        // Members
        uuid id;
        StringParam name = StringParam("name", "Name", "", "Text");
        FloatParam tempo = FloatParam("tempo", "Tempo", 120.f, "Text");
        shared_ptr<AudioContext> context;
        shared_ptr<Playhead> playhead;
        vector<shared_ptr<TrackNode>> tracks;
        vector<Scene> scenes;
        vector<MidiClipType> clips;
        shared_ptr<AnalyserNode> output;

        // Methods

        // Adds a track to the session
        shared_ptr<TrackNode> addTrack();

        // Deletes a track from the session
        void deleteTrack();

        // Gets a track by index.
        shared_ptr<TrackNode> getTrackByIndex(int);
        // Changes the order of the tracks
        void reorderTrack(int startIndex, int endIndex);

        // Gets the currently selected Track
        shared_ptr<TrackNode> selectedTrack();
        shared_ptr<TrackNode> selectTrack(int);
        Scene selectScene(int);
        Scene selectedScene();

        // Get the index of the currently selected track
        int selectedTrackIndex() const {return m_selectedTrackIndex;};

        // Get the index of the currently selected scene
        int selectedSceneIndex() const { return m_selectedSceneIndex; };

        // Create a new scene
        Scene addScene();

        // Navigate to next track in the vector, selecting it. Will loop to
        // beginning if end is reached
        void nextTrack();

        // Navigate to previous track in vector, selecting it. Will loop to
        // beginning if end is reached
        void prevTrack();

        // Navigate to next scene in vector. Will loop to beginning if end is reached
        void nextScene();

        // Navigate to previous scene in vector. Will loop to beginning if end
        // is reached
        void prevScene();

        // Delete scene
        void deleteScene(); 

        // Select gridPosition by int trackIndex, int sceneIndex
        void selectPosition(int, int);

        // Activate gridPosition by pair<int trackIndex, int sceneIndex>
        void activatePosition(std::pair<int, int>);
        
        // Activate gridPosition by int trackIndex, int sceneIndex
        void activatePosition(int, int);

        // Select gridPosition by pair<int trackIndex, int sceneIndex>
        void selectPosition(std::pair<int, int>);


        vector<reference_wrapper<const MidiClipType>> getClipsInTrack(int);
        vector<reference_wrapper<const MidiClipType>> getClipsInScene(int);

        // Create a clip in the current position
        MidiClipType& addClip();

        // Create a new clip in the selected location with the length(int bars) provided as an argument
        MidiClipType& newClip(int);

        // Delete clip in the current position
        void deleteClip();

        // Deletes clip in the provided position.
        void deleteClipAtPosition(std::pair<int, int>); 

        // Delete clip by clips index
        void deleteClipAtIndex(int);

        // Select clip by the clip index. This index is the index of the clips vector
        MidiClipType& selectClipByIndex(int);

        // Gets the clip at the provided position std::pair<int trackIndex, int sceneIndex>. Return nullptr if no clip is found.
        MidiClipType& selectClipByPosition(std::pair<int, int> clipPosition);

        // Gets the clip at the provided position using int trackIndex, int sceneIndex
        MidiClipType& selectClipByPosition(int trackIndex, int sceneIndex);

        // Returns bool if the current clip position is selected, uses position
        bool isClipSelected(std::pair<int, int>);

        // Returns bool if the current clip position is selected, uses int trackIndex, int sceneIndex
        bool isClipSelected(int, int);

        // Gets the clip at the current track and scene index.
        // If no clip is present, returns nullptr.
        MidiClipType& selectedClip();

        // Activates the clip at sceneIndex. This stops all other clips in the track.
        void activateClip(MidiClipType&, ClipState);
    private:

        // Members
        int m_selectedTrackIndex; 
        int m_selectedSceneIndex; 
        int m_selectedClipIndex; 
        MidiClipType midiClipNull = MidiClipType(nullptr);
        // Methods
        void precountState();
        void recordingState();
        void playingState();
        void onPlayheadStateChange(any state);

    };

}

#endif // !SESSION_H

