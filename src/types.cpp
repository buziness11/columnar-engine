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
        default: {
            DLOG(ERROR) << "forgot type";
            throw std::exception();
        }
    }
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
    } else {
        DLOG(ERROR) << "Cannot support this type: " << s;
        throw std::exception();
    }
}

Types StringToType(std::string&& s) {
    return StringToType(s);
}
