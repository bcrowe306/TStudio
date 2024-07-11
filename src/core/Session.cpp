#include "LabSound/LabSound.h"
#include "LabSound/extended/Logging.h"
#include "core/MidiClip.h"
#include "core/SamplerNode.h"
#include "core/Session.h"
#include "library/Browser.h"
#include "ui/Colors.h"
#include <algorithm>
#include <any>
#include <cstdlib>
#include <memory>
#include <optional>
#include <utility>
#include <functional>
#include "library/Utility.h"

using namespace placeholders;
namespace tstudio {

using MidiClipType = shared_ptr<MidiClip>;

Session::Session(shared_ptr<AudioContext> context,
                 shared_ptr<Playhead> playhead)
    : context(context), playhead(playhead), id(GenerateUUID()), EventBase() {
  // Preallocate Scene and Track vectors
  scenes.reserve(64);
  tracks.reserve(64);
  clips.reserve(4096);
  auto HOME = (std::string)getenv("HOME") + "/Documents";
  this->browser = make_shared<Browser>(HOME);

      // Setup Main output for tracks
      output = make_shared<AnalyserNode>(*context);
  context->connect(context->destinationNode(), output);

  // Subscribe to playhead state events
  eventRegistry.subscribe("playhead.state",
                          std::bind(&Session::onPlayheadStateChange, this, _1));

  // Setup initial track and scene
  for (size_t i = 0; i < 4; i++) {
    addScene();
  }
  auto newTrack = addTrack();

  // Demo code to setup InstrumentTrack
  auto sampler =
      make_shared<SamplerNode>(context, "assets/BVKER - The Astro Perc 08.wav");
  newTrack->set_instrument(sampler);

    }

    shared_ptr<TrackNode> Session::addTrack(){
        trackCounter++;
        auto newTrackNumber = std::to_string(trackCounter);
        auto colorIndex = rand() % (int)COLOR_MAP.size();
        auto newTrack = make_shared<TrackNode>(context, playhead);
        newTrack->color.set(COLOR_MAP[colorIndex]);
        newTrack->name.set(newTrackNumber + " Ins Track");
        tracks.emplace_back(newTrack);

        // Add track to context and sync
        context->connect(output, newTrack->output);
        context->synchronizeConnections();

        // Get trackIndex and set
        auto newTrackIndex = tracks.size() - 1;
        auto trackPtr = selectTrack(newTrackIndex);
        eventRegistry.notify("session.track_created", newTrackIndex);

        return newTrack;
    }

    void Session::deleteTrack(int trackIndex){
        auto track = getTrackByIndex(trackIndex);
        deleteTrack(track);
    };

    void Session::deleteTrack(shared_ptr<TrackNode> track){
        if(track != nullptr){
            // Remove clips from the track
            // auto it = std::find(tracks.begin(), tracks.end(), track);
            auto it = std::find_if(tracks.begin(), tracks.end(),[&track](shared_ptr<TrackNode> &t){
                return t->id == track->id;
            });
            auto trackIndex = it - tracks.begin();
            if(tracks.size() > 1){
                selectTrack(trackIndex -1);

                // Delete all clips in the track
                for (auto &[sceneIndex, clip] : track->clips)
                {
                    track->deleteClip(sceneIndex);
                }

                // Remove track from audio graph context and sync
                context->disconnect(output, track->output);
                context->synchronizeConnections();

                // Remove track from tracks vector
                tracks.erase(it);
            }
            
            
            
        }
       
    };

    void Session::nextTrack(){
        m_selectedTrackIndex = (m_selectedTrackIndex + 1 == tracks.size()) ? 0 : m_selectedTrackIndex + 1;
        auto track = selectTrack(m_selectedTrackIndex);
        std::cout << m_selectedTrackIndex << " : " << track->name.value << std::endl;
    };
    void Session::prevTrack(){
        m_selectedTrackIndex = (m_selectedTrackIndex - 1 < 0) ? tracks.size() - 1 : m_selectedTrackIndex - 1;
        auto track = selectTrack(m_selectedTrackIndex);
        std::cout << m_selectedTrackIndex << " : " << track->name.value << std::endl;
    };

    Scene Session::selectedScene(){
        return scenes[m_selectedSceneIndex];
    };

    void Session::nextScene(){
      auto sceneLength = scenes.size();
      m_selectedSceneIndex = (m_selectedSceneIndex + 1 == sceneLength) ? 0 : m_selectedSceneIndex + 1;
      auto scene = selectScene(m_selectedSceneIndex);
      std::cout << m_selectedSceneIndex << " : " << scene.name.value << std::endl;
    };
    void Session::prevScene(){
      m_selectedSceneIndex = (m_selectedSceneIndex - 1 < 0) ? scenes.size() - 1 : m_selectedSceneIndex - 1;
      auto scene = selectScene(m_selectedSceneIndex);
      std::cout << m_selectedSceneIndex << " : " << scene.name.value << std::endl;
    };
    Scene Session::addScene()
    {
        auto newSceneIndex = std::to_string(scenes.size() + 1);
        auto newScene = Scene();
        newScene.name.set("Scene " + newSceneIndex);
        scenes.push_back(newScene);
        m_selectedSceneIndex = scenes.size() - 1;
        eventRegistry.notify("session.scene_create", m_selectedSceneIndex);
        return selectedScene();
    }
    void Session::deleteScene(){
        Scene scene = scenes[m_selectedSceneIndex];
        try
        {
          scenes.erase(scenes.begin() + m_selectedSceneIndex);
          eventRegistry.notify("session.scene_deleted", m_selectedSceneIndex);
        }
        catch (const std::exception&)
        {
            LOG_ERROR("Scene deletion failed");
        }
    }
    void Session::selectPosition(int trackIndex, int sceneIndex) {
        selectTrack(trackIndex);
        selectScene(sceneIndex);
    };
    void Session::selectPosition(std::pair<int, int> position) {
        selectTrack(position.first);
        selectScene(position.second);
    };

    Scene Session::selectScene(int index) {
        if(index >= scenes.size() || index < 0){
            m_selectedSceneIndex = 0;
        }else{
            m_selectedSceneIndex = index;
        }
        eventRegistry.notify("session.scene_selected", m_selectedSceneIndex);
        return scenes[m_selectedSceneIndex];
    };

    void Session::activatePosition(int trackIndex, int sceneIndex) {
        selectPosition(trackIndex, sceneIndex); 
        auto track = selectTrack(trackIndex);
        auto clip = track->selectClip(sceneIndex);

        if (clip == midiClipNull)
        {
            
            track->createClip(sceneIndex);
            track->activateClip(sceneIndex, ClipState::RECORDING);
            playhead->record();
        }
        else
        {
            std::cout << "Here\n";
            track->activateClip(sceneIndex, ClipState::PLAYING);
            playhead->play();
        }
    };

    MidiClipType& Session::addClip(){

      auto currentTrack = selectedTrack();
      auto clipNumber =
          std::to_string(getClipsInTrack(m_selectedTrackIndex).size() + 1);
      if (selectedClip() == MidiClipType(nullptr)) {
        auto &clip = clips.emplace_back(make_shared<MidiClip>(
            this->playhead, currentTrack->name.value + " " + clipNumber,
            m_selectedTrackIndex, m_selectedSceneIndex));
        m_selectedClipIndex = clips.size() - 1;
        clip->addOutputNode(currentTrack);
        clip->color.set(currentTrack->color.value);
        eventRegistry.notify("session.clip_create", m_selectedClipIndex);
        return clip;
      }
      return midiClipNull;
    };
    MidiClipType& Session::newClip(int length){

        auto currentTrack = selectedTrack();
        auto clipNumber = std::to_string(getClipsInTrack(m_selectedTrackIndex).size() + 1);
        if (selectedClip() == MidiClipType(nullptr))
        {
            auto &clip =clips.emplace_back(make_shared<MidiClip>(this->playhead, currentTrack->name.value + " " + clipNumber, m_selectedTrackIndex, m_selectedSceneIndex, length));
            m_selectedClipIndex = clips.size() - 1;
            clip->addOutputNode(currentTrack);
            clip->color.set(currentTrack->color.value);
            eventRegistry.notify("session.clip_create", m_selectedClipIndex);
            return clip;
        }
        return midiClipNull;

    };

    void Session::onPlayheadStateChange(std::any data)
    {
        auto state = std::any_cast<PlayheadState>(data);
        if (state == PlayheadState::PRECOUNT){
            
            precountState();
        }
        else if (state == PlayheadState::RECORDING){
            
            recordingState();
        }
        else if (state == PlayheadState::PLAYING)
        {
            playingState();
        }
    }
    
    void Session::deleteClip(){
        auto clip = selectedClip();
        if(clip != nullptr){
            try
            {
                auto currentTrack = selectedTrack();
                clip->removeOutputNode(currentTrack);
                auto it = std::find(clips.begin(), clips.end(), clip);
                clips.erase(it);
            }
            catch (const std::exception&)
            {
                LOG_ERROR("No Clips selected");
            }
        }
    }
    void Session::deleteClip(MidiClipType clip){
        if(clip != nullptr){
            try
            {
                auto currentTrack = selectedTrack();
                clip->removeOutputNode(currentTrack);
                auto it = std::find(clips.begin(), clips.end(), clip);
                clips.erase(it);
            }
            catch (const std::exception&)
            {
                LOG_ERROR("Error deleting clips");
            }
        }
    }
    void Session::deleteClipAtPosition(std::pair<int, int> clipPosition){
        auto clip = selectClipByPosition(clipPosition);
        auto track = selectTrack(clipPosition.first);
        track->deleteClip(clipPosition.second);
    }
    void Session::deleteClipAtIndex(int index){
        try
        {
          auto &clip = clips.at(index);
          auto it = std::find(clips.begin(), clips.end(), clip);
          clips.erase(it);
        }
        catch (const std::exception&)
        {
            LOG_ERROR("Index: of clips vector is out of range");
        }
        
    }
    void Session::playingState()
    {
        for(auto &track: tracks){
            for (auto &[sceneIndex, clip] : track->clips)
            {
                auto state = clip->getState();
                std::cout << "setting clips states to playing\n"
                          << ClipStateMap[state] << std::endl;

                // If the clips are in a recording state
                if (state == ClipState::RECORDING || state == ClipState::RECORDING_INITIAL)
                {
                    track->activateClip(sceneIndex, ClipState::PLAYING);
                }
            }
        }
        
    }
    

    void Session::precountState()
    {
        auto track = selectedTrack();
        auto sceneIndex = selectedSceneIndex();
        if (track != nullptr && track->arm.get())
        {
            auto clip = track->selectClip(sceneIndex);
            if (clip == midiClipNull)
            {
                track->createClip(selectedSceneIndex());
                track->activateClip(sceneIndex, ClipState::RECORDING);
            }
            else
            {
                track->activateClip(sceneIndex, ClipState::RECORDING);
            }
        }
    }
    void Session::recordingState()
    {
        auto track = selectedTrack();
        auto sceneIndex = selectedSceneIndex();
        if (track != nullptr && track->arm.get())
        {
            auto clip = track->selectClip(sceneIndex);
            if (clip == midiClipNull)
            {
                track->createClip(selectedSceneIndex());
                track->activateClip(sceneIndex, ClipState::RECORDING);
            }
            else
            {
                track->activateClip(sceneIndex, ClipState::RECORDING);
            }
        }
    }

    shared_ptr<TrackNode> Session::selectTrack(int index){
        shared_ptr<TrackNode> track;
        if(index >= tracks.size() || index < 0){
            m_selectedTrackIndex = 0;
        }else{
            m_selectedTrackIndex = index;
        }
        track = tracks[m_selectedTrackIndex];

        for (auto t : tracks) {
            (t->id == track->id) ? t->arm.set(true) : t->arm.set(false);
        }
        return track;
    };
    shared_ptr<TrackNode> Session::getTrackByIndex(int index)
    {
        return tracks[index];
    };

    void Session::reorderTrack(int oldIndex, int newIndex)
    {
        // If old and new are the same, do nothing.
        if(oldIndex != newIndex){
            // Check index is in vector bounds before reorder. Fail silently if out of vector bounds.
            if(oldIndex >=0 && oldIndex < tracks.size() && newIndex >=0 && newIndex < tracks.size()){
                printf("Reorder: old: %d, new: %d \n", oldIndex, newIndex);
                std::lock_guard<std::mutex> lg(mtx);
                reorder_vector(this->tracks, oldIndex, newIndex);
                selectTrack(newIndex);
            }

        }
    };

    shared_ptr<TrackNode> Session::selectedTrack(){
        return tracks[m_selectedTrackIndex];
    };

    MidiClipType  Session::selectClipByIndex(int index) {
        auto &clip = clips[index];
        if(clip != nullptr){
            m_selectedClipIndex = index;
            return selectedClip();
        }else{
          return midiClipNull;
        }
    };

    MidiClipType Session::selectedClip(){
        auto track = selectedTrack();
        if(track != nullptr){
            return track->selectClip(m_selectedSceneIndex);

        }else{

        }
    };

    MidiClipType Session::selectClipByPosition(std::pair<int, int> clipPosition) {
        auto track = selectTrack(clipPosition.first);
        return track->selectClip(clipPosition.second);
    };

    void Session::duplicateClip(std::pair<int, int> clipPosition){
        return duplicateClip(clipPosition.first, clipPosition.second);
    };

    void Session::duplicateClip(int trackIndex, int sceneIndex)
    {
        int newSceneIndex = sceneIndex + 1;
        if (newSceneIndex >= scenes.size())
        {
            addScene();
        }
        auto track = tracks[trackIndex];
        if(track != nullptr){
            if (track->duplicateClip(sceneIndex, newSceneIndex)){
                selectScene(newSceneIndex);
            }
        }
    };

    MidiClipType Session::selectClipByPosition(int trackIndex, int sceneIndex)
    {
        auto track = selectTrack(trackIndex);
        return track->selectClip(sceneIndex);
    };

    bool Session::isClipSelected(std::pair<int, int> clipPosition) {
        return m_selectedTrackIndex == clipPosition.first && m_selectedSceneIndex == clipPosition.second;
    };

    bool Session::isClipSelected(int trackIndex, int sceneIndex){
        return m_selectedTrackIndex == trackIndex && m_selectedSceneIndex == sceneIndex;
    };
    vector<reference_wrapper<const MidiClipType>> Session::getClipsInTrack(int trackIndex){
        vector<reference_wrapper<const MidiClipType>> clipsInTrack;
        
        std::for_each(clips.begin(), clips.end(), [&](const MidiClipType &clip){
            if(clip->getPosition().first == trackIndex){
              clipsInTrack.push_back(std::cref<const MidiClipType>(clip));
            }
        });
        return clipsInTrack;
    }

    vector<reference_wrapper<const MidiClipType>> Session::getClipsInScene(int sceneIndex){
        vector<reference_wrapper<const MidiClipType>> clipsInScene;
        
        std::for_each(clips.begin(), clips.end(), [&](const MidiClipType &clip){
            if(clip->getPosition().second == sceneIndex){
              clipsInScene.push_back(std::cref<const MidiClipType>(clip));
            }
        });
        return clipsInScene;
    }
    void Session::changeClipPosition(std::pair<int, int> sourcePosition, std::pair<int, int> destinationPosition) {
        //  TODO: Implement move functionality
        // Validate sourcePosition is a valid clip.
        // If a clip exists in destination, delete it. This will overwrite 
        // Change the clip position on the clip item
        //Disconnect from previous track
    };
}