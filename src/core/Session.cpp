#include "LabSound/LabSound.h"
#include "LabSound/extended/Logging.h"
#include "core/SamplerNode.h"
#include "core/Session.h"
#include <algorithm>
#include <any>
#include <utility>

using namespace placeholders;
namespace tstudio {
    Session::Session(shared_ptr<AudioContext> context, shared_ptr<Playhead> playhead)
        : context(context), playhead(playhead), id( GenerateUUID() ), EventBase()
    {
        // Preallocate Scene and Track vectors
        scenes.reserve(16);
        tracks.reserve(16);
        clips.reserve(256);

        // Setup Main output for tracks
        output = make_shared<AnalyserNode>(*context);
        context->connect(context->destinationNode(), output);

        // Subscribe to playhead state events
        eventRegistry.subscribe("playhead.state", std::bind(&Session::onPlayheadStateChange, this, _1));

        // Setup initial track and scene
        for (size_t i = 0; i < 4; i++)
        {
            addScene();
        }
        auto newTrack = addTrack();

        // Demo code to setup InstrumentTrack
        auto sampler = make_shared<SamplerNode>(context, "assets/BVKER - The Astro Perc 08.wav");
        newTrack->set_instrument(sampler);

    }

    shared_ptr<TrackNode> Session::addTrack(){
        auto newTrackNumber = std::to_string(tracks.size() + 1);
        auto newTrack = make_shared<TrackNode>(context);
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
        auto index = selectedTrackIndex();
        auto trackLength = tracks.size();
        auto newIndex = (index + 1 == trackLength) ? 0 : index + 1;
        m_selectedTrackIndex = newIndex;
        auto track = selectTrack(newIndex);
        std::cout << newIndex << " : " << track->name.value << std::endl;
    };
    void Session::prevTrack(){
        auto index = selectedTrackIndex();
        auto trackLength = tracks.size();
        auto newIndex = (index - 1 < 0) ? trackLength-1 : index - 1;
        m_selectedTrackIndex = newIndex;
        auto track = selectTrack(newIndex);
        std::cout << newIndex << " : " << track->name.value << std::endl;
    };

    Scene Session::selectedScene(){
        return scenes[m_selectedSceneIndex];
    };

    void Session::nextScene(){
      auto index = selectedSceneIndex();
      auto sceneLength = scenes.size();
      auto newIndex = (index + 1 == sceneLength) ? 0 : index + 1;
      m_selectedSceneIndex = newIndex;
      auto scene = selectScene(m_selectedSceneIndex);
      std::cout << m_selectedSceneIndex << " : " << scene.name.value << std::endl;
    };
    void Session::prevScene(){
      auto index = selectedSceneIndex();
      auto sceneLength = scenes.size();
      auto newIndex = (index - 1 < 0) ? sceneLength - 1 : index - 1;
      m_selectedSceneIndex = newIndex;
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

    Scene Session::selectScene(int index) {
      bool isValidIndex = true;

      try {
        auto scene = scenes.at(index);
      } catch (const std::exception &) {
        isValidIndex = false;
      }

      if (isValidIndex) {
        m_selectedSceneIndex = index;
        return scenes[m_selectedSceneIndex];
      } else {
        LOG_ERROR("Invalid Scene Index. Returning index 0");
        m_selectedSceneIndex = 0;
      }

      eventRegistry.notify("session.scene_selected", m_selectedSceneIndex);
      return scenes[m_selectedSceneIndex];
    };
    shared_ptr<MidiClip> Session::addClip(){

        auto currentTrack = selectedTrack();
        auto clipNumber = std::to_string(clips.size()+ 1);
        auto trackIndex = selectedTrackIndex();
        auto sceneIndex = selectedSceneIndex();
        auto clip = make_shared<MidiClip>(this->playhead, currentTrack->name.value + " " + clipNumber, trackIndex, sceneIndex);
        clips.emplace_back(clip);
        m_selectedClipIndex = clips.size() -1;
        clip->addOutputNode(currentTrack);
        eventRegistry.notify("session.clip_create", m_selectedClipIndex);
        return clip;
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
    void Session::precountState()
    {
        auto clip = selectedClip();
        auto track = selectedTrack();
        
        if (clip == nullptr)
        {
            auto newClip = addClip();
            
            newClip->setState(ClipState::LAUNCHING_RECORDING);
        }
        else
        {
            clip->setNextClipState(ClipState::LAUNCHING_RECORDING);
        }
    }
    void Session::deleteClip(){
        auto clip = selectedClip();
        if(clip != nullptr){
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
    void Session::deleteClipAtPosition(std::pair<int, int> clipPosition){
        auto clip = selectClipByPosition(clipPosition);
        if(clip != nullptr){
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
          auto clip = clips.at(index);
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
        for(auto clip : clips){
            auto state = clip->getState();
            std::cout << "setting clips states to playing\n" << ClipStateMap[state] << std::endl;

            // If the clips are in a recording state
            if(state == ClipState::RECORDING || state == ClipState::RECORDING_INITIAL){
                if(playhead->launchQuantization != LaunchQuantization::Off){
                    clip->setNextClipState(ClipState::PLAYING);
                }
                else {
                    clip->setState(ClipState::PLAYING);
                }
            }
        }
    }
    void Session::recordingState()
    {
        auto clip = selectedClip();
        auto track = selectedTrack();
        if (clip == nullptr)
        {
            clip = addClip();
            clip->setState(ClipState::LAUNCHING_RECORDING);
            clip->setNextClipState(ClipState::RECORDING_INITIAL);
        }
        else
        {
            clip->setNextClipState(ClipState::RECORDING);
        }
    }
    shared_ptr<TrackNode> Session::selectTrack(int index){
        auto track = tracks[index];

        if(track == nullptr){
            m_selectedTrackIndex = 0;
            track = tracks[m_selectedTrackIndex];
        }else{
            m_selectedTrackIndex = index;
        }

        for (auto t: tracks){
            (t->id == track->id) ? t->arm.set(true) : t->arm.set(false);
        }
        return track;
    };
    

    shared_ptr<TrackNode> Session::selectedTrack(){
        return tracks[m_selectedTrackIndex];
    };

    shared_ptr<MidiClip> Session::selectClipByIndex(int index) {
        auto clip = clips[index];
        if(clip != nullptr){
            m_selectedClipIndex = index;
            return selectedClip();
        }else{
            return nullptr;
        }
    };

    shared_ptr<MidiClip> Session::selectedClip(){
      for (auto c : clips) {
        auto clipPosition = c->getPosition();
        if (clipPosition.first == m_selectedTrackIndex &&
            clipPosition.second == m_selectedSceneIndex) {
          return c;
        }
      }
      return nullptr;
    };
    shared_ptr<MidiClip> Session::selectClipByPosition(std::pair<int, int> clipPosition) {
      for (auto c : clips) {
        if (clipPosition.first == m_selectedTrackIndex &&
            clipPosition.second == m_selectedSceneIndex) {
          return c;
        }
      }
      return nullptr;
    };
}