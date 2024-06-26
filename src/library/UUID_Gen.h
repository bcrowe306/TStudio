#pragma once

#include "uuid.h"
#include <optional>
using namespace uuids;
using namespace std;

static inline uuid GenerateUUID(){
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{generator};

    return gen();
};

static inline uuid GenerateFromString(string uuid_string)
{
    auto result =  uuid::from_string(uuid_string);
    if(result.has_value()){
        return result.value();
    }else{
        return uuid{};
    }
};

static inline string UUIDToString(uuid id)
{
    return uuids::to_string(id);
};
