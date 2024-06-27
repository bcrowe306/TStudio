#ifndef SESSIONSCENE_H
#define SESSIONSCENE_H

#include <string>
#include <utility>
#pragma once
class SessionScene
{
public:
    SessionScene(const std::string &name = "", float tempo = 0.0f, const std::pair<int, int> &time_sig = {0, 0});
    std::string getName() const;
    float getTempo() const;
    std::pair<int, int> getTimeSig() const;

private:
    std::string name;
    float tempo;
    std::pair<int, int> time_sig;
};

#endif // SESSIONSCENE_H
