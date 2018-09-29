#pragma once

#include <Atrc/Common.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

AGZ_NS_BEG(Atrc)

class Ray
{
public:

    Vec3r origin;
    Vec3r direction;

    Real minT, maxT;

    Ray(const Vec3r &ori, const Vec3r &dir, Real minT = Real(0), Real maxT = RealT::Infinity())
        : origin(ori), direction(dir), minT(minT), maxT(maxT)
    {

    }

    Vec3r At(Real t) const
    {
        return origin + t * direction;
    }

    Ray Normalize() const
    {
        return Ray(origin, direction.Normalize(), minT, maxT);
    }
};

AGZ_NS_END(Atrc)
