#ifndef SONGPOSITION_H
#define SONGPOSITION_H

#include <utility>
namespace tstudio {
    
    struct SongPosition {
        int bar;
        int beat;
        int tick;
        int tpqn = 480;
        std::pair<int, int> time_sig{4,4};
        int getBeatValue() { return (tpqn * (4 / time_sig.second)); }
        static int getTickCountFromSongPosition (int bar, int beat, int tick, int tpqn, std::pair<int, int> time_sig){
          int beatValue = (tpqn * (4 / time_sig.second));
          int count = 0;
          count += bar * (beatValue * time_sig.first);
          count += (beat * tpqn);
          count += tick;
          return count;
        }
        static SongPosition getFromTick(int totalTicks, int tpqn = 480, std::pair<int, int> time_sig = std::pair<int, int>{4,4}){
          int beatValue = (tpqn * (4 / time_sig.second));
          auto barTickLenth = beatValue * time_sig.first;
          auto barCount = totalTicks / barTickLenth;
          auto beatCount = totalTicks % barTickLenth / tpqn;
          auto tickCount = totalTicks % tpqn;
          return SongPosition{
              barCount, beatCount, tickCount, tpqn, time_sig,
          };
        }
        void setFromTick(int &totalTicks){
          auto barTickLenth = getBeatValue() * time_sig.first;
          bar = totalTicks / barTickLenth;
          beat = totalTicks % barTickLenth / tpqn;
          tick = totalTicks % tpqn;
          
        }
        int getTickCount(){
            int count = 0;
            count += bar * (getBeatValue() * time_sig.first);
            count += (beat * tpqn);
            count += tick;
            return count;
        }
    };
}

#endif // !SONGPOSITION_H