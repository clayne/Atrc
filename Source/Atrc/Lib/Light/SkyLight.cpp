﻿#include <Atrc/Lib/Core/Scene.h>
#include <Atrc/Lib/Light/SkyLight.h>
#include <Texture/SphereMap.h>

namespace Atrc
{

SkyLight::SkyLight(const Spectrum &topAndBottom)
    : SkyLight(topAndBottom, topAndBottom)
{

}

SkyLight::SkyLight(const Spectrum &top, const Spectrum &bottom)
    : top_(top), bottom_(bottom), worldRadius_(0)
{

}

void SkyLight::PreprocessScene(const Scene &scene)
{
    auto wb = scene.GetWorldBound();
    worldCentre_ = (wb.high + wb.low) / 2;
    worldRadius_ = Real(1.01) * (wb.high - worldCentre_).Length();
}

Light::SampleWiResult SkyLight::SampleWi(
    const Intersection &inct, const ShadingPoint &shd, const Vec3 &sample) const noexcept
{
    auto [sam, pdf] = AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::Transform(sample.xy());
    sam = shd.coordSys.Local2World(sam);

    SampleWiResult ret;
    ret.pos      = inct.pos + 2 * sam * worldRadius_;
    ret.wi       = sam;
    ret.radiance = NonAreaLe(Ray(ret.pos, sam, EPS));
    ret.pdf      = pdf;
    ret.isDelta  = false;
    ret.isInf    = true;

    if(!ret.pdf)
        ret.radiance = Spectrum();

    return ret;
}

Real SkyLight::SampleWiAreaPDF(
    [[maybe_unused]] const Vec3 &pos, [[maybe_unused]] const Vec3 &nor,
    [[maybe_unused]] const Intersection &inct, [[maybe_unused]] const ShadingPoint &shd) const noexcept
{
    return 0;
}

Real SkyLight::SampleWiNonAreaPDF(
    const Vec3 &wi, [[maybe_unused]] const Intersection &inct, const ShadingPoint &shd) const noexcept
{
    Vec3 lwi = shd.coordSys.World2Local(wi).Normalize();
    return AGZ::Math::DistributionTransform::
        ZWeightedOnUnitHemisphere<Real>::PDF(lwi);
}

Light::SampleWiResult SkyLight::SampleWi(const Vec3 &pos, const Vec3 &sample) const noexcept
{
    auto[sam, pdf] = AGZ::Math::DistributionTransform::
        UniformOnUnitSphere<Real>::Transform(sample.xy());

    SampleWiResult ret;
    ret.pos      = pos + 2 * sam * worldRadius_;
    ret.wi       = sam;
    ret.radiance = NonAreaLe(Ray(ret.pos, sam, EPS));
    ret.pdf      = pdf;
    ret.isDelta  = false;
    ret.isInf    = true;

    return ret;
}

Real SkyLight::SampleWiAreaPDF(
    [[maybe_unused]] const Vec3 &pos, [[maybe_unused]] const Vec3 &nor,
    [[maybe_unused]] const Vec3 &medPos) const noexcept
{
    return 0;
}

Real SkyLight::SampleWiNonAreaPDF([[maybe_unused]] const Vec3 &wi, [[maybe_unused]] const Vec3 &medPos) const noexcept
{
    return AGZ::Math::DistributionTransform::UniformOnUnitSphere<Real>::PDF();
}

Spectrum SkyLight::AreaLe([[maybe_unused]] const Intersection &inct) const noexcept
{
    return Spectrum();
}

Spectrum SkyLight::NonAreaLe(const Ray &r) const noexcept
{
    Real topWeight = Real(0.5) * r.d.Normalize().z + Real(0.5);
    return topWeight * top_ + (1 - topWeight) * bottom_;
}

} // namespace Atrc