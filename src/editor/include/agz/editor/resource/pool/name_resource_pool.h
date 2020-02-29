#pragma once

#include <agz/editor/resource/pool/name_resource_pool_ui.h>
#include <agz/editor/resource/resource.h>

AGZ_EDITOR_BEGIN

class Editor;

template<typename TracerObject>
class NameResourcePool : public QObject, public ResourcePool<TracerObject>
{
public:

    NameResourcePool(ObjectContext &obj_ctx, Editor *editor, const QString &default_type);

    std::unique_ptr<ResourceReference<TracerObject>> select_resource() override;

    ResourceInPool<TracerObject> *add_resource(
        const QString &name, std::unique_ptr<ResourcePanel<TracerObject>> panel) override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

    ResourceInPool<TracerObject> *name_to_rsc(const QString &name) override;

    bool is_valid_name(const QString &name) const override;

    QString to_valid_name(const QString &name) const override;

    void show_edit_panel(ResourcePanel<TracerObject> *rsc, bool display_rsc_panel) override;

    QWidget *get_widget() override;

private:

    struct Record
    {
        QString name;
        std::unique_ptr<ResourceInPool<TracerObject>> rsc;
    };

    QString default_type_;
    ObjectContext &obj_ctx_;
    Editor *editor_ = nullptr;

    std::map<QString, std::unique_ptr<Record>> name2record_;

    NameResourcePoolWidget *widget_ = nullptr;
};

AGZ_EDITOR_END