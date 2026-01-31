#include "types.h"

std::string TypeToString(Types t) {
    switch (t) {
        case Types::kInt64_t: {
            return "int64";
        }
        case Types::kString: {
            return "string";
        }
        default: {
            return "string";
        }
    }
}

Types StringToType(std::string& s) {
    if (s == "int64") {
        return Types::kInt64_t;
    } else if (s == "string") {
        return Types::kString;
    } else {
        DLOG(WARNING) << "Cannot support such type: " << s
                      << " will work like string";
        return Types::kString;
    }
}
