﻿
#include <agz/tracer/core/bsdf.h>
#include <agz/tracer/core/entity.h>
#include <agz/tracer/core/light.h>
#include <agz/tracer/core/scene.h>

#include "./mis_light_bsdf.h"

AGZ_TRACER_BEGIN

Spectrum mis_sample_area_light(
    const Scene &scene, const AreaLight *light,
    const EntityIntersection &inct, const ShadingPoint &shd,
    const Sample5 &sam)
{
    auto light_sample = light->sample(inct.pos, sam);
    if(!light_sample.radiance || !light_sample.pdf)
        return Spectrum();

    real shadow_ray_len = (light_sample.spt.pos - inct.pos).length() - EPS;
    if(shadow_ray_len <= EPS)
        return Spectrum();
    Vec3 inct_to_light = (light_sample.spt.pos - inct.pos).normalize();
    Ray shadow_ray(inct.pos, inct_to_light, EPS, shadow_ray_len);
    if(scene.has_intersection(shadow_ray))
        return Spectrum();

    auto bsdf_f = shd.bsdf->eval(inct_to_light, inct.wr, TM_Radiance);
    if(!bsdf_f)
        return Spectrum();

    Spectrum f = light_sample.radiance * bsdf_f * std::abs(cos(inct_to_light, inct.geometry_coord.z));

    if(light_sample.is_delta)
        return f / light_sample.pdf;
    real bsdf_pdf = shd.bsdf->pdf(inct_to_light, inct.wr, TM_Radiance);
    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_bsdf(
    const Scene &scene, const EntityIntersection &inct, const ShadingPoint &shd, const Sample3 &sam)
{
    auto bsdf_sample = shd.bsdf->sample(inct.wr, TM_Radiance, sam);
    if(!bsdf_sample.f || !bsdf_sample.pdf)
        return Spectrum();

    Ray new_ray(inct.pos, bsdf_sample.dir.normalize(), EPS);
    EntityIntersection new_inct;
    if(scene.closest_intersection(new_ray, &new_inct))
    {
        if(auto light = new_inct.entity->as_light())
        {
            Spectrum light_f = light->radiance(new_inct, new_inct.wr);
            if(!light_f)
                return Spectrum();

            Spectrum f = light_f * bsdf_sample.f * std::abs(cos(inct.geometry_coord.z, bsdf_sample.dir));
            if(bsdf_sample.is_delta)
                return f / bsdf_sample.pdf;

            real light_pdf = light->pdf(inct.pos, new_inct);
            return f / (bsdf_sample.pdf + light_pdf);
        }

        return Spectrum();
    }

    return Spectrum();
}

Spectrum mis_sample_area_light(const Scene &scene, const AreaLight *light, const ScatteringPoint &pnt, const Sample5 sam)
{
    Vec3 pos = pnt.pos();

    auto light_sample = light->sample(pos, sam);
    if(!light_sample.radiance || light_sample.pdf < EPS)
        return Spectrum();

    if(!scene.visible(light_sample.spt.pos, pos))
        return Spectrum();

    Vec3 pnt_to_light = (light_sample.spt.pos - pos).normalize();
    auto bsdf_f = pnt.eval(pnt_to_light, pnt.wr(), TM_Radiance);
    if(!bsdf_f)
        return Spectrum();

    auto medium = pnt.medium(pnt_to_light);
    Spectrum tr = medium->tr(light_sample.spt.pos, pos);

    Spectrum f = tr * light_sample.radiance * bsdf_f * pnt.proj_wi_factor(pnt_to_light);
    if(light_sample.is_delta)
        return f / light_sample.pdf;

    real pnt_pdf = pnt.pdf(pnt_to_light, pnt.wr(), TM_Radiance);
    return f / (light_sample.pdf + pnt_pdf);
}

Spectrum mis_sample_envir_light(
    const Scene &scene, const EnvirLight *env,
    const EntityIntersection &inct, const ShadingPoint &shd,
    const Sample3 &sam)
{
    auto light_sample = env->sample(inct.pos, sam);
    if(!light_sample.radiance || !light_sample.pdf)
        return Spectrum();

    if(!scene.visible_inf(inct.pos, light_sample.ref_to_light))
        return {};

    auto bsdf_f = shd.bsdf->eval(light_sample.ref_to_light, inct.wr, TM_Radiance);
    if(!bsdf_f)
        return Spectrum();

    Spectrum f = light_sample.radiance * bsdf_f * std::abs(cos(light_sample.ref_to_light, inct.geometry_coord.z));

    if(light_sample.is_delta)
        return f / light_sample.pdf;
    real bsdf_pdf = shd.bsdf->pdf(light_sample.ref_to_light, inct.wr, TM_Radiance);
    return f / (light_sample.pdf + bsdf_pdf);
}

Spectrum mis_sample_envir_light(const Scene &scene, const EnvirLight *env, const ScatteringPoint &pnt, const Sample3 sam)
{
    Vec3 pos = pnt.pos();

    auto light_sample = env->sample(pos, sam);
    if(!light_sample.radiance || light_sample.pdf < EPS)
        return {};

    if(!scene.visible_inf(pos, light_sample.ref_to_light))
        return {};

    Vec3 pnt_to_light = light_sample.ref_to_light.normalize();
    auto bsdf_f = pnt.eval(pnt_to_light, pnt.wr(), TM_Radiance);
    if(!bsdf_f)
        return Spectrum();

    Spectrum f = light_sample.radiance * bsdf_f * pnt.proj_wi_factor(pnt_to_light);
    if(light_sample.is_delta)
        return f / light_sample.pdf;

    real pnt_pdf = pnt.pdf(pnt_to_light, pnt.wr(), TM_Radiance);
    return f / (light_sample.pdf + pnt_pdf);
}

Spectrum mis_sample_light(
    const Scene &scene, const Light *lht,
    const EntityIntersection &inct, const ShadingPoint &shd,
    const Sample5 &sam)
{
    if(lht->is_area())
        return mis_sample_area_light(scene, lht->as_area(), inct, shd, sam);
    return mis_sample_envir_light(scene, lht->as_envir(), inct, shd, { sam.u, sam.v, sam.w });
}

Spectrum mis_sample_light(const Scene &scene, const Light *lht, const ScatteringPoint &pnt, const Sample5 sam)
{
    if(lht->is_area())
        return mis_sample_area_light(scene, lht->as_area(), pnt, sam);
    return mis_sample_envir_light(scene, lht->as_envir(), pnt, { sam.u, sam.v, sam.w });
}

Spectrum mis_sample_scattering(const Scene &scene, const ScatteringPoint &pnt, const Sample3 &sam)
{
    auto pnt_sample = pnt.sample(pnt.wr(), sam, TM_Radiance);
    if(!pnt_sample.f || pnt_sample.pdf < EPS)
        return Spectrum();

    Ray new_ray(pnt.pos(), pnt_sample.dir.normalize(), EPS);
    EntityIntersection new_inct;
    if(!scene.closest_intersection(new_ray, &new_inct))
    {
        auto env = scene.env();
        Spectrum light_f = env->radiance(pnt.pos(), pnt_sample.dir);

        Spectrum f = light_f * pnt_sample.f * pnt.proj_wi_factor(pnt_sample.dir);
        if(pnt_sample.is_delta)
            return f / pnt_sample.pdf;

        real light_pdf = env->pdf(pnt.pos(), pnt_sample.dir);
        return f / (pnt_sample.pdf + light_pdf);
    }

    auto light = new_inct.entity->as_light();
    if(!light)
        return Spectrum();

    Spectrum light_f = light->radiance(new_inct, new_inct.wr);
    if(!light_f)
        return Spectrum();

    auto medium = pnt.medium(pnt_sample.dir);
    Spectrum tr = medium->tr(pnt.pos(), new_inct.pos);

    Spectrum f = tr * light_f * pnt_sample.f * pnt.proj_wi_factor(pnt_sample.dir);
    if(pnt_sample.is_delta)
        return f / pnt_sample.pdf;

    real light_pdf = light->pdf(pnt.pos(), new_inct);
    return f / (pnt_sample.pdf + light_pdf);
}

AGZ_TRACER_END