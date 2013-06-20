/*
 * Copyright (C) 2013 Canonical Ltd
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
 * Authored by: Robert Carr <robert.carr@canonical.com>
 */

#include "mircommon/event_helpers_mir.h"

#include "window_mirserver_priv.h"
#include "application_instance_mirserver_priv.h"
#include "window_properties_mirserver_priv.h"


#include <mir/shell/surface.h>
#include <mir/input/input_platform.h>
#include <mir/input/input_receiver_thread.h>
#include <mir/graphics/internal_client.h>

namespace uams = ubuntu::application::mir::server;
namespace uaum = ubuntu::application::ui::mir;

namespace mg = mir::graphics;
namespace msh = mir::shell;
namespace mircv = mir::input::receiver;

namespace
{

static void ua_ui_window_handle_event(UAUiWindowInputEventCb cb, void* ctx, MirEvent* mir_event)
{
    Event ubuntu_ev;
    uaum::event_to_ubuntu_event(mir_event, ubuntu_ev);
    cb(ctx, &ubuntu_ev);
}

}

uams::Window::Window(uams::Instance& instance, uams::WindowProperties* properties,
                     std::shared_ptr<mircv::InputPlatform> const& input_platform,
                     std::shared_ptr<mg::InternalClient> const& internal_client)
    : window_properties(properties),
      surface(instance.create_surface(properties->surface_parameters())),
      internal_client(internal_client)
{
    input_thread = input_platform->create_input_thread(surface->client_input_fd(),
        std::bind(ua_ui_window_handle_event, properties->input_cb(), properties->input_context(), std::placeholders::_1));
    input_thread->start();
}

uams::Window::~Window()
{
    input_thread->stop();
    input_thread->join();
}

UAUiWindow* uams::Window::as_u_window()
{
    return static_cast<UAUiWindow*>(this);
}

uams::Window* uams::Window::from_u_window(UAUiWindow *u_window)
{
    return static_cast<uams::Window*>(u_window);
}

EGLNativeWindowType uams::Window::get_native_type()
{
    return internal_client->egl_native_window(surface);
}
