#include <Atrc/Camera/ApertureCamera.h>

AGZ_NS_BEG(Atrc)

ApertureCamera::ApertureCamera(
    const Vec3r &eye, const Vec3r &_dir, const Vec3r &up,
    Radr FOVy, Real aspectRatio, Real apertureRadius, Real focusDis)
    : eye_(eye), dir_(_dir.Normalize()), scrCen_(AGZ::UNINITIALIZED),
      scrX_(AGZ::UNINITIALIZED), scrY_(AGZ::UNINITIALIZED),
      scrXDir_(AGZ::UNINITIALIZED), scrYDir_(AGZ::UNINITIALIZED),
      apertureRadius_(apertureRadius), focusDis_(focusDis)
{
    scrCen_ = eye_ + dir_;

    scrXDir_ = Cross(dir_, up).Normalize();
    scrYDir_ = Cross(scrXDir_, dir_);

    Real scrYSize = Tan(Real(0.5) * FOVy);
    Real scrXSize = scrYSize * aspectRatio;

    scrX_ = scrXSize * scrXDir_;
    scrY_ = scrYSize * scrYDir_;
}

namespace
{
    Vec2r UniformlySampleInUnitDisk()
    {
        for(;;)
        {
            Vec2r ret(2 * Rand() - 1, 2 * Rand() - 1);
            if(ret.LengthSquare() <= 1)
                return ret;
        }
    }
}

Ray ApertureCamera::GetRay(const Vec2r &screenSample) const
{
    Vec3r ori = scrCen_ + screenSample.x * scrX_ + screenSample.y * scrY_;
    Vec3r dir = (ori - eye_).Normalize();
    Real t = focusDis_ / Dot(dir, dir_);
    Vec3r focusPoint = eye_ + t * dir;

    Vec2r sam = apertureRadius_ * UniformlySampleInUnitDisk();
    Vec3r newOri = scrCen_ + scrXDir_ * sam.x + scrYDir_ * sam.y;

    return Ray(newOri, (focusPoint - newOri).Normalize());
}

AGZ_NS_END(Atrc)