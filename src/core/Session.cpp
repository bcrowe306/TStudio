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
        auto trackIndex = (int)tracks.size();
        auto newTrackNumber = std::to_string(tracks.size() + 1);
        auto colorIndex = rand() % (int)COLOR_MAP.size();
        auto newTrack = make_shared<TrackNode>(context);
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
        Scene scene;
        try
        {
            scene = scenes.at(index);
        }
        catch(const std::exception& e)
        {
            m_selectedSceneIndex = 0;
            return scene;
        }
        m_selectedSceneIndex = index;
        eventRegistry.notify("session.scene_selected", m_selectedSceneIndex);
        return scenes[m_selectedSceneIndex];
    };

    void Session::activatePosition(int trackIndex, int sceneIndex) {
        selectPosition(trackIndex, sceneIndex); 
        auto &clip = selectedClip();

        if (clip == midiClipNull)
        {

            auto nClip = addClip();
            activateClip(nClip, ClipState::RECORDING);
            playhead->record();
        }
        else
        {
            switch (clip->getState())
            {
            case ClipState::PLAYING:
                activateClip(clip, ClipState::PLAYING);
                break;
            case ClipState::RECORDING:
                activateClip(clip, ClipState::PLAYING);
                break;
            case ClipState::RECORDING_INITIAL:
                activateClip(clip, ClipState::PLAYING);
                break;
            case ClipState::STOPPED:
                activateClip(clip, ClipState::PLAYING);
                break;
            
            default:
                break;
            }
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
        auto &clip = selectedClip();
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
    void Session::deleteClipAtPosition(std::pair<int, int> clipPosition){
        auto &clip = selectClipByPosition(clipPosition);
        if(clip != midiClipNull){
            try
            {
                auto it = std::find(clips.begin(), clips.end(), clip);
                clips.erase(it);
            }
            catch (const std::exception&)
            {
                LOG_ERROR("No Clips selected");
            }
        }
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
        for(auto &clip : clips){
            auto state = clip->getState();
            std::cout << "setting clips states to playing\n" << ClipStateMap[state] << std::endl;

            // If the clips are in a recording state
            if(state == ClipState::RECORDING || state == ClipState::RECORDING_INITIAL){
                activateClip(clip, ClipState::PLAYING);
            }
        }
    }
    void Session::recordingState()
    {
        if (selectedTrack()->arm.get())
        {
            auto &clip = selectedClip();
            if (clip == midiClipNull){
              auto nClip = addClip();
              activateClip(nClip, ClipState::RECORDING);
            }else{
                activateClip(clip, ClipState::RECORDING);
            }
        }
    }

    void Session::precountState()
    {
        // Create a new clip and start recording if track is armed:
        if(selectedTrack()->arm.get()){
            auto &clip = selectedClip();
            if (clip == midiClipNull)
            {
                auto nClip = addClip();
                activateClip(nClip, ClipState::RECORDING);
            }else{
                activateClip(clip, ClipState::RECORDING);
            }

        }
    }

    shared_ptr<TrackNode> Session::selectTrack(int index){
        auto track = tracks[index];

        if(track == nullptr){
            m_selectedTrackIndex = 0;
            track = tracks[m_selectedTrackIndex];
        }else{
            m_selectedTrackIndex = index;
            for (auto t : tracks) {
              (t->id == track->id) ? t->arm.set(true) : t->arm.set(false);
                
            }
        }

        
        return track;
    };
    shared_ptr<TrackNode> Session::getTrackByIndex(int index)
    {
        return tracks[index];
    };

    shared_ptr<TrackNode> Session::selectedTrack(){
        return tracks[m_selectedTrackIndex];
    };

    MidiClipType& Session::selectClipByIndex(int index) {
        auto &clip = clips[index];
        if(clip != nullptr){
            m_selectedClipIndex = index;
            return selectedClip();
        }else{
          return midiClipNull;
        }
    };

    MidiClipType& Session::selectedClip(){
      for (auto &c : clips) {
        auto clipPosition = c->getPosition();
        if (clipPosition.first == m_selectedTrackIndex &&
            clipPosition.second == m_selectedSceneIndex) {
          return c;
        }
      }
      return midiClipNull;
    };
    MidiClipType& Session::selectClipByPosition(std::pair<int, int> clipPosition) {
      for (auto &c : clips) {
        if (c->getPosition() == clipPosition) {
          return c;
        }
      }
      return midiClipNull;
    };

    MidiClipType &Session::selectClipByPosition(int trackIndex, int sceneIndex)
    {
        for (auto &c : clips)
        {
            auto cPos = c->getPosition();
            if (cPos.first == trackIndex && cPos.second == sceneIndex)
            {
                return c;
            }
        }
        return midiClipNull;
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
    void Session::activateClip(MidiClipType &activeClip, ClipState state)
    {
        auto activePosition = activeClip->getPosition();
        for(auto &clip: clips){
            auto cPos = clip->getPosition();
            // Only get clips is the track specified by activePosition
            if(cPos.first == activePosition.first){
                if(cPos.second != activePosition.second){
                    clip->setNextClipState(ClipState::STOPPED);
                }else{
                    clip->setNextClipState(state);
                }
            }
        }
    }
}