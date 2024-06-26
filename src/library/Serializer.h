#pragma once
#include <iostream>
#include "yaml-cpp/yaml.h"

class Serializer
{
private:
    /* data */
public:
    Serializer(/* args */);
    ~Serializer();
    virtual void Serialize(YAML::Emitter &out, std::string nodeKey = "") = 0;

    virtual void Deserialize(YAML::Node & yaml) = 0;
    void printSerialization()
    {
        YAML::Emitter out;
        out << YAML::BeginDoc;
        out << YAML::BeginMap;
        this->Serialize(out, "test_print");
        std::cout << out.c_str() << std::endl;
    }
};

