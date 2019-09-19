﻿#include <agz/tracer/core/medium.h>
#include "./phase_function.h"

AGZ_TRACER_BEGIN

class HomogeneousMedium : public Medium
{
    Spectrum sigma_a_; // 吸收系数
    Spectrum sigma_s_; // 散射系数
    Spectrum sigma_t_; // 衰减系数
    real g_ = 0;       // 散射不对称因子

    Spectrum albedo() const
    {
        return !sigma_t_ ? Spectrum(1) : sigma_s_ / sigma_t_;
    }

public:

    static std::string description()
    {
        return R"___(
homogeneous [Medium]
    sigma_a [Spectrum] absorbtion
    sigma_s [Spectrum] scattering
    g       [real]     asymmetry
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext&) override
    {
        AGZ_HIERARCHY_TRY

        init_customed_flag(params);

        sigma_a_ = params.child_spectrum("sigma_a");
        sigma_s_ = params.child_spectrum("sigma_s");
        sigma_t_ = sigma_a_ + sigma_s_;

        g_ = params.child_real("g");
        if(g_ <= -1 || g_ >= 1)
            throw ObjectConstructionException("invalid g value: " + std::to_string(g_));

        AGZ_HIERARCHY_WRAP("in initializing homogeneous medium")
    }

    Spectrum tr(const Vec3 &a, const Vec3 &b) const noexcept override
    {
        Spectrum exp = -sigma_t_ * (a - b).length();
        return {
            std::exp(exp.r),
            std::exp(exp.g),
            std::exp(exp.b)
        };
    }

    SampleMediumResult sample(const Vec3 &o, const Vec3 &d, real t_min, real t_max, const Sample1 &sam) const noexcept override
    {
        // 这里scale一下t_min/t_max，就好像d的长度是1一样，最后算位置的时候再scale回来就行了
        real len = d.length();
        t_min *= len;
        t_max *= len;

        if(!sigma_s_)
            return SAMPLE_MEDIUM_RESULT_INVALID;

        real t_interval = t_max - t_min;
        assert(t_interval > 0);

        auto [channel, new_sam] = math::distribution::extract_uniform_int(sam.u, 0, SPECTRUM_COMPONENT_COUNT);
        real st = -std::log(new_sam) / sigma_t_[channel];

        bool sample_medium = st < t_interval;
        Spectrum tr;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            tr[i] = std::exp(-sigma_t_[i] * (std::min)(st, t_interval));
        Spectrum density = sample_medium ? sigma_s_ * tr : tr;

        real pdf = 0;
        for(int i = 0; i < SPECTRUM_COMPONENT_COUNT; ++i)
            pdf += density[i];
        pdf /= SPECTRUM_COMPONENT_COUNT;
        pdf = (std::max)(pdf, EPS);

        SampleMediumResult ret;

        if(!sample_medium)
        {
            ret.pdf = pdf;
            return ret;
        }

        ret.inct.medium = this;
        ret.inct.t      = (st + t_min) / len;
        ret.inct.wr     = -d;
        ret.pdf         = pdf;
        ret.inct.pos    = o + d * ret.inct.t;

        return ret;
    }

    ShadingPoint shade(const MediumIntersection &inct, Arena &arena) const noexcept override
    {
        assert(!inct.invalid());
        auto bsdf = arena.create<HenyeyGreensteinPhaseFunction>(g_, sigma_s_, albedo());
        return { bsdf };
    }
};

AGZT_IMPLEMENTATION(Medium, HomogeneousMedium, "homogeneous")

AGZ_TRACER_END