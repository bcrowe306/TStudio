#include <iostream>
#include <fstream>
#include "daw/Project.h"
#include "daw/InstrumentTrack.h"
#include <functional>
#include "devices/DeviceMap.h"

void Project::init(){
    // Build the inital graph
    this->graph = new AudioGraph();
    this->metronome = new Metronome();
    this->graph->addNode(&(metronome->beat));
    this->graph->addNode(&(metronome->downBeat));
};
Project::~Project()
{
    delete this->metronome;
    delete this->graph;
};

void Project::save(std::string filePath){
    // Create and open a text file
    ofstream ProjectFile(filePath);

    // Start the YAML Emitter
    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap;
    // Write to the file
    this->Serialize(out);
    ProjectFile << out.c_str();

    // Close the file
    ProjectFile.close();
};

void Project::Serialize(YAML::Emitter &out, string nodeKey) {

    out << YAML::Key << "Project";
    out << YAML::Value;
    out << YAML::BeginMap;

    out << YAML::Key << "id";
    out << YAML::Value << UUIDToString(this->id);
    name.Serialize(out, "name");
    tempo.Serialize(out, "tempo");
    preCountBars.Serialize(out, "preCountBars");
    inputQuantize.Serialize(out, "inputQuantize");
    quantizeValue.Serialize(out, "quantizeValue");
    quantizeStart.Serialize(out, "quantizeStart");
    quantizeEnd.Serialize(out, "quantizeEnd");
    quantizeStrength.Serialize(out, "quantizeStrength");
    launchQuantization.Serialize(out, "launchQuantization");
    metronome->Serialize(out, "metronome");

    // Tracks
    out << YAML::Key << "tracks";
    out << YAML::Value;
    out << YAML::BeginSeq;
    for(auto &track : tracks){
        track->Serialize(out);
    }
}
void Project::Deserialize(YAML::Node &baseNode) {
    if (baseNode["Project"])
    {
        auto node = baseNode["Project"];
        auto nameNode = node["name"];
        this->name.Deserialize(nameNode);
        if (node["id"])
        {
            id = GenerateFromString(node["id"].as<string>());
        }
        if (node["name"])
        {
            auto nameNode = node["name"];
            this->name.Deserialize(nameNode);
        }
        if (node["tempo"])
        {
            auto tempoNode = node["tempo"];
            this->tempo.Deserialize(tempoNode);
        }
        if (node["preCountBars"])
        {
            auto preCountBarsNode = node["preCountBars"];
            this->preCountBars.Deserialize(preCountBarsNode);
        }
        if (node["inputQuantize"])
        {
            auto inputQuantizeNode = node["inputQuantize"];
            this->inputQuantize.Deserialize(inputQuantizeNode);
        }
        if (node["quantizeValue"])
        {
            auto quantizeValueNode = node["quantizeValue"];
            this->quantizeValue.Deserialize(quantizeValueNode);
        }
        if (node["quantizeStart"])
        {
            auto quantizeStartNode = node["quantizeStart"];
            this->quantizeStart.Deserialize(quantizeStartNode);
        }
        if (node["quantizeEnd"])
        {
            auto quantizeEndNode = node["quantizeEnd"];
            this->quantizeEnd.Deserialize(quantizeEndNode);
        }
        if (node["quantizeStrength"])
        {
            auto quantizeStrengthNode = node["quantizeStrength"];
            this->quantizeStrength.Deserialize(quantizeStrengthNode);
        }
        if (node["launchQuantization"])
        {
            auto launchQuantizationNode = node["launchQuantization"];
            this->launchQuantization.Deserialize(launchQuantizationNode);
        }
        if (node["tracks"])
        {
            auto tracks = node["tracks"];
            for (std::size_t i = 0; i < tracks.size(); i++)
            {
                auto track = tracks[i];
                auto new_track = shared_ptr<InstrumentTrack>(new InstrumentTrack());
                new_track->Deserialize(track);
                this->graph->addNode(new_track);
            }
            auto launchQuantizationNode = node["launchQuantization"];
            this->launchQuantization.Deserialize(launchQuantizationNode);
        }
    }else{
        return;
    }
    
}

InsTrack Project::addInstrumentTrack()
{
    auto trackIndex = this->graph->children.size();

    auto new_track = InsTrack(new InstrumentTrack());
    new_track->name.set(trackIndex + " Ins Track");

    this->graph->addNode(new_track);
    return new_track;
};
void Project::removeInstrumentTrack(uuid id){
    this->graph->removeNode(id);
};

InsTrack Project::addDeviceToTrack(InsTrack track, string deviceName)
{
    auto device = DeviceMap[deviceName]();
    track->setPlugin(device);
}

