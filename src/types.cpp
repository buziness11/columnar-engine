#include "types.h"
#include <exception>

std::string TypeToString(Types t) {
    switch (t) {
        case Types::kInt16_t: {
            return "int16";
        }
        case Types::kInt32_t: {
            return "int32";
        }
        case Types::kInt64_t: {
            return "int64";
        }
        case Types::kString: {
            return "string";
        }
        case Types::kDate: {
            return "DATE";
        }
        case Types::kTimestamp: {
            return "TIMESTAMP";
        }
        case Types::kDouble: {
            return "double";
        }
        case Types::kLongDouble: {
            return "long double";
        }
        case Types::kBool: {
            return "bool";
        }
    }
    DLOG(ERROR) << "forgot add type in type to string";
    throw std::exception();
}

Types StringToType(const std::string& s) {
    if (s == "int16") {
        return Types::kInt16_t;
    } else if (s == "int32") {
        return Types::kInt32_t;
    } else if (s == "int64") {
        return Types::kInt64_t;
    } else if (s == "string") {
        return Types::kString;
    } else if (s == "DATE") {
        return Types::kDate;
    } else if (s == "TIMESTAMP") {
        return Types::kTimestamp;
    } else if (s == "double") {
        return Types::kDouble;
    } else if (s == "long double") {
        return Types::kLongDouble;
    } else if (s == "bool") {
        return Types::kBool;
    }
    DLOG(ERROR) << "forgot add type: " << s;
    throw std::exception();
}

Types StringToType(std::string&& s) {
    return StringToType(s);
}
