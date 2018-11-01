#pragma once

#include <Atrc/Core/Core.h>

AGZ_NS_BEG(Atrc)

class BVH : public Entity
{
public:

    struct Internal
    {
        AABB bound;
        uint32_t rightChild;
    };

    struct Leaf
    {
        uint32_t start, end;
    };

    using Node = AGZ::TypeOpr::Variant<Internal, Leaf>;

    using ConstEntityPtr = const Entity*;

    explicit BVH(const std::vector<const Entity*> &entities);

    BVH(const ConstEntityPtr *entities, size_t nEntity);

    bool HasIntersection(const Ray &r) const override;

    bool FindIntersection(const Ray &r, SurfacePoint *sp) const override;

    AABB WorldBound() const override;

    const Material *GetMaterial(const SurfacePoint &sp) const override;

private:

    // ���entities_��nodes_
    void InitBVH(const ConstEntityPtr *entities, uint32_t nEntity);

    std::vector<const Entity*> entities_;
    std::vector<Node> nodes_;
};

AGZ_NS_END(Atrc)