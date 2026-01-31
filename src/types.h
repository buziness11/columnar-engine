#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <glog/logging.h>

enum class Types { kInt64_t, kString };

std::string TypeToString(Types t);

Types StringToType(std::string& s);