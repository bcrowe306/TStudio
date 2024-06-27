#include <string>
#include <vector>
#include <memory>
#include "daw/TrackBase.h"
#include "audio/AudioGraph.h"
#include "daw/Metronome.h"
#include "daw/Parameter.h"
#include "app/Serializer.h"
#include "daw/InstrumentTrack.h"
#include "uuid.h"
#include <mutex>

using std::vector;
using std::string;
using std::shared_ptr;

typedef std::shared_ptr<InstrumentTrack> InsTrack;

class Project : public Serializer
{
private:
    mutable std::mutex mtx;
    /* data */
public:
    uuid id;
    vector<shared_ptr<TrackBase>> tracks;
    string filePath;
    AudioGraph *graph;
    Metronome *metronome;
    Parameter<string> name = Parameter<string>("name", "Name", "Untitled", "Text");
    Parameter<float> tempo = Parameter<float>("tempo", "Tempo", 120.0f, "Text", 20.0f, 300.0f);
    Parameter<int> preCountBars = Parameter<int>("preCountBars", "Pre Count", 1, "Text", 0, 4);
    Parameter<bool> inputQuantize = Parameter<bool>("inputQuantize", "Input Quantize", true, "Checkbox");
    Parameter<int> quantizeValue = Parameter<int>("quantizeValue", "Quantize", 16, "Select");
    Parameter<bool> quantizeStart = Parameter<bool>("quantizeStart", "Quantize Start", true, "Checkbox");
    Parameter<bool> quantizeEnd = Parameter<bool>("quantizeEnd", "Quantize End", false, "Checkbox");
    Parameter<float> quantizeStrength = Parameter<float>("quantizeStrength", "Quantize Strength", 1.0f, "Text");
    Parameter<float> launchQuantization = Parameter<float>("launchQuantization", "Launch Quantization", 1.0f, "Text");
    TimeSig time_sig{4, 4}; // TODO: Serialize Timesig
    Project():  id(GenerateUUID()){
        this->name.set("New Project");
        init();
    };
    ~Project();
    void init();
    void Serialize(YAML::Emitter &out, string nodeKey = "") override;
    void Deserialize(YAML::Node &node) override;
    void save(std::string filePath);
    InsTrack addInstrumentTrack();
    void removeInstrumentTrack(uuid id);
    InsTrack addDeviceToTrack(InsTrack track, string deviceName);
    static Project* load(string &filePath){
        auto baseNode = YAML::LoadFile(filePath);
        if(baseNode){
            auto project = new Project();
            project->Deserialize(baseNode);
            project->filePath = filePath;
            return project;
        }
        else{
            return nullptr;
        }
    };
};

