#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>
#include <functional>
#include "library/UUID_Gen.h"
#include "yaml-cpp/yaml.h"
#include "library/Serializer.h"
#include "library/EventEmitter.h"
using std::string;
using std::any;
using std::any_cast;
using std::vector;
using std::unordered_map;

template <typename Type>
struct ParameterOption
{
    string label;
    Type value;
};


template <typename Type>
class Parameter : public Serializer, public EventEmitter<any>
{

public:
    uuid id;
    string displayType;
    string name;
    string label;
    Type value;
    Type defaultValue;
    Type min;
    Type max;
    vector<ParameterOption<Type>> options;
    bool hasOptions = false;
    std::function<void(string, Type)> callback;
    Parameter() : id(GenerateUUID()){};
    Parameter(uuid id) : id(id){};
    Parameter(string id) : id(GenerateFromString(id)){};
    Parameter(string name, string label, string displayType)
        : id(GenerateUUID()), displayType(displayType), name(name), label(label){};

    Parameter(string name, string label, Type value, string displayType)
        : id(GenerateUUID()), displayType(displayType), name(name), label(label), value(value), defaultValue(value){};

    Parameter(string name, string label, Type value, string displayType, Type min, Type max)
        : id(GenerateUUID()), displayType(displayType), name(name), label(label), value(value), defaultValue(value), min(min), max(max){};
    // ~Parameter();
    Type get() {
        return value;
    };
    void set(Type value){
        this->value = value;
        if(callback){
            callback(this->name, this->value);
        }
        this->notify("changed", value);
    };
    void addOption(string label, Type value){
        this->options.push_back(ParameterOption<Type>{label, value});
    }


    void Serialize(YAML::Emitter &out, string nodeKey = "") override {
        if(nodeKey != ""){
            out << YAML::Key << nodeKey;
            out << YAML::Value;
        }
        out << YAML::BeginMap;

        out << YAML::Key << "id";
        out << YAML::Value << UUIDToString(this->id);

        out << YAML::Key << "displayType";
        out << YAML::Value << this->displayType;

        out << YAML::Key << "label";
        out << YAML::Value << this->label;

        out << YAML::Key << "name";
        out << YAML::Value << this->name;

        out << YAML::Key << "value";
        out << YAML::Value << this->value;

        out << YAML::Key << "defaultValue";
        out << YAML::Value << this->defaultValue;

        out << YAML::Key << "min";
        out << YAML::Value << this->min;

        out << YAML::Key << "max";
        out << YAML::Value << this->max;

        out << YAML::Key << "hasOptions";
        out << YAML::Value << this->hasOptions;

        if(options.size() > 0){
            out << YAML::Key << "options";
            out << YAML::Value;
            out << YAML::BeginSeq;
            for(auto &option: options){
                out << YAML::BeginMap;
                out << YAML::Key << "label";
                out << YAML::Value << option.label;
                out << YAML::Key << "value";
                out << YAML::Value << option.value;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }

        out << YAML::EndMap;
    }
    void Deserialize(YAML::Node &node) override {
        if (node["id"]){
            this->id = GenerateFromString(node["id"].as<string>());
        }
        if (node["displayType"])
        {
            this->displayType = node["displayType"].as<string>();
        }
        if (node["name"])
        {
            this->name = node["name"].as<string>();
        }
        if (node["label"])
        {
            this->label = node["label"].as<string>();
        }
        if (node["value"])
        {
            this->value = node["value"].as<Type>();
        }
        if (node["defaultValue"])
        {
            this->defaultValue = node["defaultValue"].as<Type>();
        }
        if (node["min"])
        {
            this->min = node["min"].as<Type>();
        }
        if (node["max"])
        {
            this->max = node["max"].as<Type>();
        }
        if (node["hasOptions"])
        {
            this->hasOptions = node["hasOptions"].as<bool>();
        }
        if (node["hasOptions"])
        {
            this->hasOptions = node["hasOptions"].as<bool>();
        }
        if (node["options"])
        {
            auto options = node["options"];
            for (std::size_t i = 0; i < options.size(); i++)
            {
                this->addOption(options[i]["label"].as<string>(), options[i]["value"].as<Type>());
            }
        }
        };
};

typedef Parameter<string> StringParam;
typedef Parameter<float> FloatParam;
typedef Parameter<int> IntParam;
typedef Parameter<bool> BoolParam;