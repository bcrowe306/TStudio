#include "core/SessionManager.h"
#include <iostream>

SessionManager::SessionManager(std::vector<std::shared_ptr<InstrumentTrack>> &tracks)
    : tracks(tracks), selectedCell(0, 0), record_options(std::make_shared<RecordPlayOptions>())
{
    event_registry = std::make_shared<EventRegistry>();
    createInitialScenes();
    createInitialMatrix();
    event_registry->subscribe("tracks.selected", [this](auto track)
                              { onSelectedTrack(track); });
    event_registry->subscribe("playhead.state", [this](auto state)
                              { onPlayheadStateChange(state); });
}

void SessionManager::precountState()
{
    auto cell = getCell();
    auto track = getTrackFromSelectedCell();
    if (!cell->clip)
    {
        cell->clip = createClip(track->getName() + "_" + std::to_string(selectedCell.second), ClipState::NEWRECORDING);
    }
    else
    {
        cell->setNextClipState(ClipState::LAUNCHRECORDING);
    }
}

void SessionManager::recordingState()
{
    auto cell = getCell();
    auto track = getTrackFromSelectedCell();
    if (!cell->clip)
    {
        cell->clip = createClip(track->getName() + "_" + std::to_string(selectedCell.second), ClipState::NEWRECORDING);
        cell->setNextClipState(ClipState::INITIALRECORDING);
    }
    else
    {
        cell->setNextClipState(ClipState::RECORDING);
    }
}

std::shared_ptr<MidiClip> SessionManager::createClip(const std::string &name, ClipState state)
{
    auto track = getTrackFromSelectedCell();
    auto clip = std::make_shared<MidiClip>(name, state);
    clip->setMidiOutCallback(std::bind(&InstrumentTrack::onMidiClipEvents, track, std::placeholders::_1));
    track->addClipListener(std::bind(&MidiClip::onMidiInEvent, clip, std::placeholders::_1));
    notify("clip");
    return clip;
}

void SessionManager::playingState()
{
    auto cell = getCell();
    if (cell->clip)
    {
        cell->setNextClipState(ClipState::PLAYING);
    }
}

void SessionManager::stopAllClipsInTrack()
{
    auto cells_in_track = matrix[selectedCell.first];
    for (size_t i = 0; i < cells_in_track.size(); ++i)
    {
        if (i != selectedCell.second)
        {
            cells_in_track[i]->stopClip();
        }
    }
}

void SessionManager::launchCellAction()
{
    // Implement cell launch actions and logic
}

void SessionManager::onPlayheadStateChange(PlayheadState state)
{
    switch (state)
    {
    case PlayheadState::PRECOUNT:
        precountState();
        break;
    case PlayheadState::RECORDING:
        recordingState();
        break;
    case PlayheadState::PLAYING:
        playingState();
        break;
    }
}

std::shared_ptr<InstrumentTrack> SessionManager::getTrackFromSelectedCell()
{
    return tracks[selectedCell.first];
}

void SessionManager::onSelectedTrack(std::shared_ptr<InstrumentTrack> track)
{
    selectedTrack = track;
    selectedCell = std::make_pair(track->getIndex(), selectedCell.second);
    std::cout << "(" << selectedCell.first << "," << selectedCell.second << ")" << std::endl;
}

std::shared_ptr<SessionCell> SessionManager::getCell()
{
    return matrix[selectedCell.first][selectedCell.second];
}

bool SessionManager::isCellSelectionValid(const std::pair<int, int> &position)
{
    try
    {
        auto &t = matrix.at(position.first);
        t.at(position.second);
        return true;
    }
    catch (std::out_of_range &)
    {
        return false;
    }
}

void SessionManager::selectCell(const std::pair<int, int> &position)
{
    if (isCellSelectionValid(position))
    {
        selectedCell = position;
    }
}

void SessionManager::stopCell(const std::pair<int, int> &position)
{
    if (isCellSelectionValid(position))
    {
        selectedCell = position;
        // Implement logic to stop cells, and all clips in this track
    }
}

void SessionManager::activateCell(const std::pair<int, int> &position)
{
    if (isCellSelectionValid(position))
    {
        selectedCell = position;
        // Implement Logic to either record a new clip in this cell, or play the current clip in the cell
    }
}

void SessionManager::nextCell()
{
    auto &cells = matrix[selectedCell.first];
    auto current_index = selectedCell.second;
    auto next_index = (current_index + 1 < cells.size()) ? current_index + 1 : current_index;
    selectedCell = std::make_pair(selectedCell.first, next_index);
    std::cout << "(" << selectedCell.first << "," << selectedCell.second << ")" << std::endl;
}

void SessionManager::previousCell()
{
    auto &cells = matrix[selectedCell.first];
    auto current_index = selectedCell.second;
    auto next_index = (current_index > 0) ? current_index - 1 : current_index;
    selectedCell = std::make_pair(selectedCell.first, next_index);
    std::cout << "(" << selectedCell.first << "," << selectedCell.second << ")" << std::endl;
}

void SessionManager::notify(const std::string &event_id, /*additional args*/)
{
    event_registry->notifyListeners("session." + event_id, /*additional args*/);
}

void SessionManager::createInitialScenes()
{
    for (int i = 0; i < 8; ++i)
    {
        scenes.push_back(std::make_shared<SessionScene>(std::to_string(i + 1)));
    }
}

void SessionManager::createInitialMatrix()
{
    matrix.resize(tracks.size());
    for (auto &row : matrix)
    {
        row.resize(scenes.size());
        for (auto &cell : row)
        {
            cell = std::make_shared<SessionCell>();
        }
    }
}

void SessionManager::addScene()
{
    scenes.push_back(std::make_shared<SessionScene>(std::to_string(scenes.size() + 1)));
}

void SessionManager::newClip(const std::pair<int, int> &position)
{
    try
    {
        matrix.at(position.first).at(position.second) = std::make_shared<MidiClip>("Midi Clip " + std::to_string(position.first) + "-" + std::to_string(position.second));
    }
    catch (std::out_of_range &)
    {
        std::cout << "Cell not found in matrix" << std::endl;
    }
    printMatrix();
}

void SessionManager::updateMatrix()
{
    for (size_t i = 0; i < tracks.size(); ++i)
    {
        try
        {
            auto &cells = matrix.at(i);
        }
        catch (std::out_of_range &)
        {
            matrix.push_back(std::vector<std::shared_ptr<SessionCell>>(scenes.size()));
            for (auto &cell : matrix.back())
            {
                cell = std::make_shared<SessionCell>();
            }
        }
    }
}

void SessionManager::printMatrix()
{
    for (const auto &row : matrix)
    {
        for (const auto &cell : row)
        {
            std::cout << (cell ? "O" : "X");
        }
        std::cout << std::endl;
    }
}
