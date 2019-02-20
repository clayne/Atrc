#pragma once

#include <Atrc/Lib/Material/Utility/BxDF.h>
#include <Atrc/Lib/Material/Utility/Fresnel.h>

namespace Atrc
{

// IMPROVE: anisotropic
// See https://disney-animation.s3.amazonaws.com/library/s2012_pbs_disney_brdf_notes_v2.pdf
class DisneyPrincipledBxDF : public BxDF
{
    Spectrum rc_;
    Real roughness_;
    Real specular_;
    Real specularTint_;
    Real metallic_;
    Real sheen_;
    Real sheenTint_;
    Real subsurface_;
    Real clearCoat_;
    Real clearCoatGloss_;

    Real gamma_;
    const Fresnel *fresnel_;

    Real Diffuse(const Vec3 &nWi, const Vec3 &nWo, Real cosThetaD) const;

    Real Subsurface(const Vec3 &nWi, const Vec3 &nWo, Real cosThetaD) const;

    Spectrum Specular(const Vec3 &nWi, const Vec3 &nWo, const Vec3 &nH) const;

    Real ClearCoat(const Vec3 &nWi, const Vec3 &nWo, const Vec3 &nH, Real cosThetaD) const;

public:

    DisneyPrincipledBxDF(
        const Spectrum &rc,
        Real roughness,
        Real specular,
        Real specularTint,
        Real metallic,
        Real sheen,
        Real sheenTint,
        Real subsurface,
        Real clearCoat,
        Real clearCoatGloss,
        const Fresnel *fresnel) noexcept;

    Spectrum GetAlbedo() const noexcept override;

    Spectrum Eval(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;

    std::optional<SampleWiResult> SampleWi(const CoordSystem &geoInShd, const Vec3 &wo, bool star, const Vec3 &sample) const noexcept override;

    Real SampleWiPDF(const CoordSystem &geoInShd, const Vec3 &wi, const Vec3 &wo, bool star) const noexcept override;
};

} // namespace Atrc
