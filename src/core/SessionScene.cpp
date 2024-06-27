#include "SessionScene.h"

SessionScene::SessionScene(const std::string &name, float tempo, const std::pair<int, int> &time_sig)
    : name(name), tempo(tempo), time_sig(time_sig) {}

std::string SessionScene::getName() const
{
    return name;
}

float SessionScene::getTempo() const
{
    return tempo;
}

std::pair<int, int> SessionScene::getTimeSig() const
{
    return time_sig;
}
