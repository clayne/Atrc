#include <iostream>

#include <Atrc/Atrc.h>
#include <Utils.h>

using namespace Atrc;
using namespace std;

// See https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float ACESFilm(float x)
{
    constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;
    return Clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

AGZ::Tex::Texture2D<Color3b> ToSavedImage(const RenderTarget &origin, float gamma)
{
    return origin.Map([=](const Color3f &color)
    {
        return color.Map([=](float x)
        {
            return static_cast<uint8_t>(Clamp(Pow(ACESFilm(x), gamma), 0.0f, 1.0f) * 255);
        });
    });
}

int main()
{
    constexpr uint32_t SCR_W = 1920;
    constexpr uint32_t SCR_H = 1080;
    constexpr Real SCR_ASPECT_RATIO = static_cast<Real>(SCR_W) / SCR_H;

    //============= Camera =============

    const Vec3 eye = { -5.0, 0.0, 0.0 };
    const Vec3 dir = Vec3(0.0) - eye;
    PerspectiveCamera camera(eye, dir, { 0.0, 0.0, 1.0 }, Deg(90.0f), SCR_ASPECT_RATIO);

    //============= Scene =============

    Sphere sph(Transform::Translate(0.0, 0.0, -101.0), 100.0);
    PureDiffuse groundMat(Spectrum(0.4f, 0.7f, 0.9f));
    GeometryEntity ground(&sph, &groundMat);

    Sphere sph2(Transform::Translate(0.0, 0.0, 1.0), 1.0);
    GeometricLightEntity<GeometricDiffuseLight> sphere(&sph2, Spectrum(3.0f));

    Scene scene;
    scene.camera = &camera;
    scene.entities_ = { &ground, &sphere };

    for(auto ent : scene.entities_)
    {
        auto L = ent->AsLight();
        if(L)
            scene.lights_.push_back(L);
    }

    //============= Render Target =============

    RenderTarget renderTarget(SCR_W, SCR_H);

    //============= Renderer & Integrator =============

    JitteredSubareaRenderer subareaRenderer(100);

    ParallelRenderer renderer(6);
    //SerialRenderer renderer;
    renderer.SetProgressPrinting(true);

    //PureColorIntegrator integrator(SPECTRUM::WHITE, SPECTRUM::BLACK);
    PathTracer integrator(10);

    //============= Rendering =============

    cout << "Start rendering..." << endl;

    AGZ::Timer timer;
    renderer.Render(subareaRenderer, scene, integrator, renderTarget);
    auto deltaTime = timer.Milliseconds() / 1000.0;

    cout << "Complete rendering...Total time: " << deltaTime << "s." << endl;

    //============= Output =============

    AGZ::Tex::TextureFile::WriteRGBToPNG("./Build/Output.png", ToSavedImage(renderTarget, 1 / 2.2f));
}
