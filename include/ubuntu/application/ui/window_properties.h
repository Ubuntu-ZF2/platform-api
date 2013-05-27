/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Ricardo Mendoza <ricardo.mendoza@canonical.com>
 *              Thomas Voß <thomas.voss@canonical.com>           
 */

#ifndef UBUNTU_APPLICATION_UI_WINDOW_PROPERTIES_H_
#define UBUNTU_APPLICATION_UI_WINDOW_PROPERTIES_H_

#include <ubuntu/application/ui/input/event.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum {
        U_MAIN_ROLE = 1,
        U_DASH_ROLE,
        U_INDICATOR_ROLE,
        U_NOTIFICATIONS_ROLE,
        U_GREETER_ROLE,
        U_LAUNCHER_ROLE,
        U_ON_SCREEN_KEYBOARD_ROLE,
        U_SHUTDOWN_DIALOG_ROLE,
    } UAUiWindowRole;


    typedef void (*input_event_cb)(void* ctx, const Event* ev);
    typedef input_event_cb UAUiWindowInputEventCb;

    typedef void UAUiWindowProperties;
    
    UAUiWindowProperties*
    ua_ui_window_properties_new_for_normal_window();
   
    void
    ua_ui_window_properties_destroy(
    	UAUiWindowProperties *properties);
    
    void
    ua_ui_window_properties_set_titlen(
    	UAUiWindowProperties *properties,
    	const char *title,
        size_t size);
    
    const char*
    ua_ui_window_properties_get_title(
    	UAUiWindowProperties *properties);
  
    void
    ua_ui_window_properties_set_role(
        UAUiWindowProperties *properties,
        UAUiWindowRole role);

    UAUiWindowRole
    ua_ui_window_properties_get_role(
        UAUiWindowProperties *properties);
   
    void
    ua_ui_window_properties_set_input_cb_and_ctx(
        UAUiWindowProperties *properties,
        UAUiWindowInputEventCb cb,
        void *ctx);
    
#ifdef __cplusplus
}
#endif

#endif /* UBUNTU_APPLICATION_UI_WINDOW_PROPERTIES_H_ */
