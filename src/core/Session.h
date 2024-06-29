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
#include <vector>
#include <string>
#include <memory>
#include <any>

using namespace lab;
using std::vector;
using std::string;
using std::shared_ptr;
using std::make_shared;
using std::any;

// TODO: Implement Undo/Redo - Command Pattern

namespace tstudio {
    struct Scene : public EventBase {
        Scene() :id(GenerateUUID()), EventBase() {}
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
        vector<shared_ptr<MidiClip>> clips;
        shared_ptr<AnalyserNode> output;

        // Methods
        shared_ptr<TrackNode> addTrack();
        void deleteTrack();
        void reorderTrack(int startIndex, int endIndex);
        shared_ptr<TrackNode> selectedTrack();
        shared_ptr<TrackNode> selectTrack(int);
        Scene& selectScene(int); // TODO: Implement
        int selectedTrackIndex() const {return m_selectedTrackIndex;};
        int selectedSceneIndex() const { return m_selectedSceneIndex; };
        Scene& addScene();
        void deleteScene(); // TODO: Implement
        shared_ptr<MidiClip> addClip();
        shared_ptr<MidiClip> deleteClip(); // TODO: Implement
        shared_ptr<MidiClip> selectClipByIndex(int);

        // Gets the clip at the current track and scene index.
        // If no clip is present, returns nullptr.
        shared_ptr<MidiClip> selectClipByPosition();
        shared_ptr<MidiClip> selectedClip();
    private:

        // Members
        int m_selectedTrackIndex; 
        int m_selectedSceneIndex; 
        int m_selectedClipIndex; 

        // Methods
        void precountState();
        void recordingState();
        void playingState();
        void onPlayheadStateChange(any state);
    };

}

#endif // !SESSION_H

