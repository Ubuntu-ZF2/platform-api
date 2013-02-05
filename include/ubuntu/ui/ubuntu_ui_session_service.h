#ifndef UBUNTU_UI_SESSION_SERVICE_C_API_H_
#define UBUNTU_UI_SESSION_SERVICE_C_API_H_

#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
    {
        UNKNOWN_APP = -1,
        CAMERA_APP = 0,
        GALLERY_APP = 1,
        BROWSER_APP = 2,
        SHARE_APP = 3,
        TELEPHONY_APP = 4
    } ubuntu_ui_well_known_application;

    typedef const void* ubuntu_ui_session_properties;
    typedef const void* ubuntu_ui_session_preview_provider;

    typedef void (*ubuntu_ui_session_service_snapshot_cb)(const void* pixels, unsigned int width, unsigned int height, unsigned int stride, void* context);

    typedef void (*session_requested_cb)(ubuntu_ui_well_known_application app, void* context);
    typedef void (*session_born_cb)(ubuntu_ui_session_properties props, void* context);
    typedef void (*session_unfocused_cb)(ubuntu_ui_session_properties props, void* context);
    typedef void (*session_focused_cb)(ubuntu_ui_session_properties props, void* context);
    typedef void (*session_died_cb)(ubuntu_ui_session_properties props, void * context);

    typedef struct
    {
        session_requested_cb on_session_requested;
        session_born_cb on_session_born;
        session_unfocused_cb on_session_unfocused;
        session_focused_cb on_session_focused;
        session_died_cb on_session_died;

        void* context;
    } ubuntu_ui_session_lifecycle_observer;

    const char* 
    ubuntu_ui_session_properties_get_value_for_key(
        ubuntu_ui_session_properties props, 
        const char* key);

    int 
    ubuntu_ui_session_properties_get_application_instance_id(
        ubuntu_ui_session_properties props);

    const char* 
    ubuntu_ui_session_properties_get_desktop_file_hint(
        ubuntu_ui_session_properties props);

    void 
    ubuntu_ui_session_install_session_lifecycle_observer(
        ubuntu_ui_session_lifecycle_observer* observer);

    int /* boolean */
    ubuntu_ui_session_preview_provider_update_session_preview_texture_with_id(
        ubuntu_ui_session_preview_provider pp,
        int id,
        GLuint texture,
        unsigned int* width,
        unsigned int* height);

    void 
    ubuntu_ui_session_unfocus_running_sessions();

    void 
    ubuntu_ui_session_focus_running_session_with_id(int id);

    void 
    ubuntu_ui_session_snapshot_running_session_with_id(int id, ubuntu_ui_session_service_snapshot_cb cb, void* context);

    void 
    ubuntu_ui_session_trigger_switch_to_well_known_application(ubuntu_ui_well_known_application app);

    void
    ubuntu_ui_report_osk_visible(int x, int y, int width, int height);
    
    void
    ubuntu_ui_report_osk_invisible();

    void
    ubuntu_ui_report_notification_visible();
    
    void
    ubuntu_ui_report_notification_invisible();

#ifdef __cplusplus
}
#endif

#endif // UBUNTU_UI_SESSION_SERVICE_C_API_H_
