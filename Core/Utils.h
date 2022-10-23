//
//  Utils.h
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 10/5/22.
//

#ifndef Utils_h
#define Utils_h

#include <cstring>

struct StrCmp {
    bool operator()(const char* lhs, const char* rhs) const {
        return std::strcmp(lhs, rhs) < 0;
    }
};

#endif /* Utils_h */
