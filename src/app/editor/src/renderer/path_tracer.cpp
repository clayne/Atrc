#include <agz/editor/renderer/path_tracer.h>

AGZ_EDITOR_BEGIN

PathTracer::PathTracer(const Params &params, int fb_width, int fb_height, std::shared_ptr<const tracer::Scene> scene)
    : PerPixelRenderer(params.worker_count, params.task_grid_size, fb_width, fb_height, 128, 32, scene)
{
    trace_params_.min_depth = params.min_depth;
    trace_params_.max_depth = params.max_depth;
    trace_params_.cont_prob = params.cont_prob;

    fast_params_.ao_sample_count        = 4;
    fast_params_.max_occlusion_distance = 1;

    fast_preview_ = params.fast_preview;
}

PathTracer::~PathTracer()
{
    stop_rendering();
}

Spectrum PathTracer::fast_render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena)
{
    if(fast_preview_)
        return trace_albedo_ao(fast_params_, scene, ray, sampler, arena).value;
    return trace_std(trace_params_, scene, ray, sampler, arena).value;
}

Spectrum PathTracer::render_pixel(const tracer::Scene &scene, const tracer::Ray &ray, tracer::Sampler &sampler, tracer::Arena &arena)
{
    return trace_std(trace_params_, scene, ray, sampler, arena).value;
}

AGZ_EDITOR_END
