#ifndef IGETLIBINFO_H
#define IGETLIBINFO_H

#include "structs.h"

class IGetLibInfo
{
public:
    IGetLibInfo() = default;

    virtual LibInfo&& getLibInfo(const QString& lib) = 0;

    virtual ~IGetLibInfo() = 0;

};

#endif // IGETLIBINFO_H
