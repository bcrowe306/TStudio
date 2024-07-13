#ifndef AUDIOUTILITY_H
#define AUDIOUTILITY_H
#include <cmath>
namespace tstudio {

inline float dBToLinear(float dB) { return std::pow(10.0f, dB / 10.0f); }
}

inline float linearToDB(float linearValue) { return 10.0f * std::log10(linearValue); }

inline float linearToPercentage(float linearValue, float maxLinearValue) {
  return (linearValue / maxLinearValue) * 100.0f;
}


#endif // !AUDIOUTILITY_H