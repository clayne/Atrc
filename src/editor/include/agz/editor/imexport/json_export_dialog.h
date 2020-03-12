#pragma once

#include <agz/editor/envir_light/envir_light.h>
#include <agz/editor/imexport/json_export_context.h>

AGZ_EDITOR_BEGIN

class GlobalSettingWidget;
class ObjectContext;
class PreviewWindow;
class RendererPanel;
class SceneManager;

void export_json(
    SceneManager        *scene_mgr,
    ObjectContext       *obj_ctx,
    EnvirLightSlot      *envir_light,
    PreviewWindow       *preview_window,
    GlobalSettingWidget *global_settings,
    RendererPanel       *renderer_panel);

AGZ_EDITOR_END
