#include <ubuntu/ui/ubuntu_ui_session_service.h>

#include <ubuntu/ui/session_service.h>
#include <ubuntu/ui/session_enumerator.h>

#include <stdio.h>

namespace
{
struct SessionLifeCycleObserver : public ubuntu::ui::SessionLifeCycleObserver
{
    SessionLifeCycleObserver(ubuntu_ui_session_lifecycle_observer* observer) : observer(observer)
    {
    }

    void on_session_requested(ubuntu::ui::WellKnownApplication app)
    {
        if (!observer)
            return;

        if (!observer->on_session_requested)
            return;

        observer->on_session_requested(static_cast<ubuntu_ui_well_known_application>(app), observer->context);
    }

    void on_session_born(const ubuntu::ui::SessionProperties::Ptr& props)
    {
        if (!observer)
            return;

        if (!observer->on_session_born)
            return;

        observer->on_session_born(&props, observer->context);
    }

    void on_session_unfocused(const ubuntu::ui::SessionProperties::Ptr& props)
    {
        if (!observer)
            return;

        if (!observer->on_session_unfocused)
            return;

        observer->on_session_unfocused(&props, observer->context);
    }

    void on_session_focused(const ubuntu::ui::SessionProperties::Ptr& props)
    {
        if (!observer)
            return;

        if (!observer->on_session_focused)
            return;

        observer->on_session_focused(&props, observer->context);
    }

    void on_session_died(const ubuntu::ui::SessionProperties::Ptr& props)
    {
        if (!observer)
            return;

        if (!observer->on_session_died)
            return;

        observer->on_session_died(&props, observer->context);
    }

    ubuntu_ui_session_lifecycle_observer* observer;
};

}

const char* ubuntu_ui_session_properties_get_value_for_key(ubuntu_ui_session_properties props, const char* key)
{
    if (!props)
        return NULL;

    if (!key)
        return NULL;

    const ubuntu::ui::SessionProperties::Ptr* p = static_cast<const ubuntu::ui::SessionProperties::Ptr*>(props);

    return (*p)->value_for_key(key);
}

int ubuntu_ui_session_properties_get_application_instance_id(ubuntu_ui_session_properties props)
{
    if (!props)
        return -1;

    const ubuntu::ui::SessionProperties::Ptr* p = static_cast<const ubuntu::ui::SessionProperties::Ptr*>(props);

    return (*p)->application_instance_id();
}

const char* ubuntu_ui_session_properties_get_desktop_file_hint(ubuntu_ui_session_properties props)
{
    if (!props)
        return NULL;

    const ubuntu::ui::SessionProperties::Ptr* p = static_cast<const ubuntu::ui::SessionProperties::Ptr*>(props);

    return (*p)->desktop_file_hint();
}

bool ubuntu_ui_session_preview_provider_update_session_preview_texture(ubuntu_ui_session_preview_provider pp, int id, GLuint texture, unsigned int* width, unsigned int* height)
{
    if (!pp)
        return false;

    const ubuntu::ui::SessionPreviewProvider::Ptr* spp =
        static_cast<const ubuntu::ui::SessionPreviewProvider::Ptr*>(pp);

    return (*spp)->get_or_update_session_preview(texture, *width, *height);
}

void ubuntu_ui_session_install_session_lifecycle_observer(ubuntu_ui_session_lifecycle_observer* observer)
{
    ubuntu::ui::SessionLifeCycleObserver::Ptr p(new SessionLifeCycleObserver(observer));
    ubuntu::ui::SessionService::instance()->install_session_lifecycle_observer(p);
}

void ubuntu_ui_session_unfocus_running_sessions()
{
    ubuntu::ui::SessionService::instance()->unfocus_running_sessions();
}

void ubuntu_ui_session_focus_running_session_with_id(int id)
{
    ubuntu::ui::SessionService::instance()->focus_running_session_with_id(id);
}

void ubuntu_ui_session_snapshot_running_session_with_id(int id, ubuntu_ui_session_service_snapshot_cb cb, void* context)
{
    ubuntu::ui::SessionSnapshot::Ptr ss = ubuntu::ui::SessionService::instance()->snapshot_running_session_with_id(id);

    if (cb)
    {
        cb(ss->pixel_data(), ss->width(), ss->height(), ss->stride(), context);
    }
}

void ubuntu_ui_session_trigger_switch_to_well_known_application(ubuntu_ui_well_known_application app)
{
    ubuntu::ui::SessionService::instance()->trigger_switch_to_well_known_application(
        static_cast<ubuntu::ui::WellKnownApplication>(app));
}

void ubuntu_ui_report_osk_visible(int x, int y, int width, int height)
{
    ubuntu::ui::SessionService::instance()->report_osk_visible(x, y, width, height);
}

void ubuntu_ui_report_osk_invisible()
{
    ubuntu::ui::SessionService::instance()->report_osk_invisible();
}

void ubuntu_ui_report_notification_visible()
{
    ubuntu::ui::SessionService::instance()->report_notification_visible();
}

void ubuntu_ui_report_notification_invisible()
{
    ubuntu::ui::SessionService::instance()->report_notification_invisible();
}
