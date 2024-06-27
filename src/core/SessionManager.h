#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "EventRegistry.h"
#include "daw/MidiClip.h"
#include "InstrumentTrack.h"
#include "RecordPlayOptions.h"
#include "SessionScene.h"
#include "SessionCell.h"
#pragma once
class SessionManager
{
public:
    SessionManager(std::vector<std::shared_ptr<InstrumentTrack>> &tracks);

    void precountState();
    void recordingState();
    std::shared_ptr<MidiClip> createClip(const std::string &name, ClipState state);
    void playingState();
    void stopAllClipsInTrack();
    void launchCellAction();

    void onPlayheadStateChange(PlayheadState state);
    std::shared_ptr<InstrumentTrack> getTrackFromSelectedCell();
    void onSelectedTrack(std::shared_ptr<InstrumentTrack> track);

    std::shared_ptr<SessionCell> getCell();
    bool isCellSelectionValid(const std::pair<int, int> &position);

    void selectCell(const std::pair<int, int> &position);
    void stopCell(const std::pair<int, int> &position);
    void activateCell(const std::pair<int, int> &position);

    void nextCell();
    void previousCell();

    void notify(const std::string &event_id, /*additional args*/);

    void createInitialScenes();
    void createInitialMatrix();
    void addScene();
    void newClip(const std::pair<int, int> &position);
    void updateMatrix();
    void printMatrix();

private:
    std::shared_ptr<EventRegistry> event_registry;
    std::vector<std::shared_ptr<InstrumentTrack>> tracks;
    std::vector<std::vector<std::shared_ptr<SessionCell>>> matrix;
    std::vector<std::shared_ptr<SessionScene>> scenes;
    std::pair<int, int> selectedCell;
    std::shared_ptr<InstrumentTrack> selectedTrack;
    std::shared_ptr<RecordPlayOptions> record_options;
};

#endif // SESSIONMANAGER_H
