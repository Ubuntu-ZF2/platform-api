/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#undef LOG_TAG
#define LOG_TAG "mir::ApplicationManager"

#include "default_application_manager.h"

#include "default_application_manager_input_setup.h"
#include "default_application_session.h"
#include "default_shell.h"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>

#include <input/InputListener.h>
#include <input/InputReader.h>
#include <androidfw/InputTransport.h>
#include <utils/threads.h>

#include <cstdio>

#include <sys/types.h>
#include <signal.h>

namespace mir
{

template<typename T, typename U>
const T& min(const T& lhs, const U& rhs)
{
    return lhs < rhs ? lhs : rhs;
}

bool is_session_allowed_to_run_in_background(
    const android::sp<mir::ApplicationSession>& session)
{
    ALOGI("%s: %s", __PRETTY_FUNCTION__, session->desktop_file.string());
    static const android::String8 telephony_app_desktop_file("/usr/share/applications/telephony-app.desktop");
    if (session->desktop_file == telephony_app_desktop_file)
    {
        return true;
    }

    return false;
}

template<int x, int y, int w, int h>
int ApplicationManager::ShellInputSetup::Window<x, y, w, h>::looper_callback(int receiveFd, int events, void* ctxt)
{
    // ALOGI("%s", __PRETTY_FUNCTION__);

    bool result = true;
    ApplicationManager::ShellInputSetup::Window<x, y, w, h>* window = static_cast<ApplicationManager::ShellInputSetup::Window<x, y, w, h>*>(ctxt);
    
    //window->input_consumer.receiveDispatchSignal();
    uint32_t seq;
    android::InputEvent* ev;
    static const bool consume_batches = true;
    static const nsecs_t frame_time = -1;
    switch(window->input_consumer.consume(&window->event_factory, consume_batches, frame_time, &seq, &ev))
    {
        case android::OK:
            result = true;
            //printf("We have a client side event for process %d. \n", getpid());
            window->input_consumer.sendFinishedSignal(seq, result);
            break;
        case android::INVALID_OPERATION:
            result = true;
            break;
        case android::NO_MEMORY:
            result = true;
            break;
    }

    return result ? 1 : 0;
}

template<int x, int y, int w, int h>
ApplicationManager::ShellInputSetup::Window<x, y, w, h>::Window(
    ApplicationManager::ShellInputSetup* parent,
    int _x,
    int _y,
    int _w,
    int _h) : parent(parent),
              input_consumer(client_channel)
{
    auto window = new android::InputSetup::DummyApplicationWindow(
        parent->shell_application,
        _x,
        _y,
        _w,
        _h);
    
    android::InputChannel::openInputChannelPair(
        android::String8("DummyShellInputChannel"),
        server_channel,
        client_channel);

    window->input_channel = server_channel;
    input_window = window;
    parent->input_manager->getDispatcher()->registerInputChannel(
        window->input_channel,
        input_window,
        false);

    input_consumer = android::InputConsumer(client_channel);
    // input_consumer.initialize();
    parent->looper->addFd(client_channel->getFd(), //client_channel->getReceivePipeFd(),
                          0,
                          ALOOPER_EVENT_INPUT,
                          looper_callback,
                          this);
}

ApplicationManager::ShellInputSetup::DisplayInfo::DisplayInfo()
{
    auto display = android::SurfaceComposerClient::getBuiltInDisplay(
        android::ISurfaceComposer::eDisplayIdMain);
    
    android::SurfaceComposerClient::getDisplayInfo(
        display,
        &info);
}

ApplicationManager::ShellInputSetup::ShellInputSetup(const android::sp<android::InputManager>& input_manager)
        : shell_has_focus(true),
          input_manager(input_manager),
          shell_application(new android::InputSetup::DummyApplication()),
          looper(new android::Looper(true)),
          event_loop(looper),
          event_trap_window(this, 0, 0, display_info.info.w, display_info.info.h),
          osk_window(this),
          notifications_window(this)
{
    event_loop.run();
}

ApplicationManager::InputFilter::InputFilter(ApplicationManager* manager) : manager(manager)
{
}

bool ApplicationManager::InputFilter::filter_event(const android::InputEvent* event)
{
    bool result = true;

    switch (event->getType())
    {
        case AINPUT_EVENT_TYPE_KEY:
            result = handle_key_event(static_cast<const android::KeyEvent*>(event));
            break;
    }

    return result;
}

bool ApplicationManager::InputFilter::handle_key_event(const android::KeyEvent* event)
{
    bool result = true;

    if (!event)
        return result;

    if (event->getAction() == AKEY_EVENT_ACTION_DOWN)
    {
        switch (event->getKeyCode())
        {
            case AKEYCODE_VOLUME_DOWN:
                manager->lock();
                manager->kill_focused_application_locked();
                manager->unlock();
                break;
        }
    }

    return result;
}

ApplicationManager::LockingIterator::LockingIterator(
    ApplicationManager* manager,
    size_t index) : manager(manager),
                    it(index)
{
}

void ApplicationManager::LockingIterator::advance()
{
    it += 1;
}

bool ApplicationManager::LockingIterator::is_valid() const
{
    return it < manager->apps.size();
}

void ApplicationManager::LockingIterator::make_current()
{
    //printf("%s \n", __PRETTY_FUNCTION__);
}

const android::sp<mir::ApplicationSession>& ApplicationManager::LockingIterator::operator*()
{
    return manager->apps.valueFor(manager->apps_as_added[it]);
}

ApplicationManager::LockingIterator::~LockingIterator()
{
    manager->unlock();
}

ApplicationManager::ApplicationManager() : input_filter(new InputFilter(this)),
                                           input_setup(new android::InputSetup(input_filter)),
                                           is_osk_visible(false),
                                           are_notifications_visible(false),
                                           focused_application(0)
{
    shell_input_setup = new ShellInputSetup(input_setup->input_manager);

    input_setup->input_manager->getDispatcher()->setFocusedApplication(
        shell_input_setup->shell_application);
    
    android::Vector< android::sp<android::InputWindowHandle> > input_windows;
    input_windows.push(shell_input_setup->event_trap_window.input_window);
    input_setup->input_manager->getDispatcher()->setInputWindows(
        input_windows);

    input_setup->start();
}

// From DeathRecipient
void ApplicationManager::binderDied(const android::wp<android::IBinder>& who)
{
    ALOGI("%s \n", __PRETTY_FUNCTION__);
    android::Mutex::Autolock al(guard);
    android::sp<android::IBinder> sp = who.promote();

    const android::sp<mir::ApplicationSession>& dead_session = apps.valueFor(sp);

    notify_observers_about_session_died(dead_session->remote_pid,
                                        dead_session->desktop_file);

    size_t i = 0;
    for(i = 0; i < apps_as_added.size(); i++)
    {
        if (apps_as_added[i] == sp)
            break;
    }

    size_t next_focused_app = 0;
    next_focused_app = apps_as_added.removeAt(i);
    apps.removeItem(sp);

    if (next_focused_app >= apps_as_added.size())
        next_focused_app = 0;

    if (i == focused_application)
    {
        switch_focused_application_locked(next_focused_app);
    }
    else if(focused_application > i)
    {
        focused_application--;
    }

}

void ApplicationManager::lock()
{
    guard.lock();
}

void ApplicationManager::unlock()
{
    guard.unlock();
}

android::sp<ApplicationManager::LockingIterator> ApplicationManager::iterator()
{
    lock();
    android::sp<ApplicationManager::LockingIterator> it(
        new ApplicationManager::LockingIterator(this, 0));

    return it;
}

void ApplicationManager::start_a_new_session(
    int32_t session_type,
    const android::String8& app_name,
    const android::String8& desktop_file,
    const android::sp<android::IApplicationManagerSession>& session,
    int fd)
{
    (void) session_type;
    android::sp<mir::ApplicationSession> app_session(
        new mir::ApplicationSession(
            android::IPCThreadState::self()->getCallingPid(),
            session,
            session_type,
            app_name,
            desktop_file));
    {
        android::Mutex::Autolock al(guard);
        session->asBinder()->linkToDeath(
            android::sp<android::IBinder::DeathRecipient>(this));
        apps.add(session->asBinder(), app_session);
        apps_as_added.push_back(session->asBinder());

        switch(session_type)
        {
            case ubuntu::application::ui::user_session_type:
                ALOGI("%s: Invoked for user_session_type \n", __PRETTY_FUNCTION__);
                break;
            case ubuntu::application::ui::system_session_type:
                ALOGI("%s: Invoked for system_session_type \n", __PRETTY_FUNCTION__);
                break;
        }
    }

    notify_observers_about_session_born(app_session->remote_pid, app_session->desktop_file);
}

void ApplicationManager::register_a_surface(
    const android::String8& title,
    const android::sp<android::IApplicationManagerSession>& session,
    int32_t surface_role,
    int32_t token,
    int fd)
{
    android::Mutex::Autolock al(guard);
    android::sp<android::InputChannel> input_channel(
        new android::InputChannel(
            title,
            dup(fd)));

    android::sp<mir::ApplicationSession::Surface> surface(
        new mir::ApplicationSession::Surface(
            apps.valueFor(session->asBinder()).get(),
            input_channel,
            surface_role,
            token));

    auto registered_session = apps.valueFor(session->asBinder());

    ALOGI("Registering input channel as observer: %s",
         registered_session->session_type == ubuntu::application::ui::system_session_type ? "true" : "false");

    input_setup->input_manager->getDispatcher()->registerInputChannel(
        surface->input_channel,
        surface->make_input_window_handle(),
        registered_session->session_type == ubuntu::application::ui::system_session_type);

    registered_session->register_surface(surface);

    if (registered_session->session_type == ubuntu::application::ui::system_session_type)
    {
        ALOGI("New surface for system session, adjusting layer now.");
        switch(surface_role)
        {
            case ubuntu::application::ui::dash_actor_role:
                registered_session->raise_surface_to_layer(token, default_dash_layer);
                break;
            case ubuntu::application::ui::indicator_actor_role:
                registered_session->raise_surface_to_layer(token, default_indicator_layer);
                break;
            case ubuntu::application::ui::notifications_actor_role:
                registered_session->raise_surface_to_layer(token, default_notifications_layer);
                break;
            case ubuntu::application::ui::greeter_actor_role:
                registered_session->raise_surface_to_layer(token, default_greeter_layer);
                break;
            case ubuntu::application::ui::launcher_actor_role:
                registered_session->raise_surface_to_layer(token, default_launcher_layer);
                break;
            case ubuntu::application::ui::on_screen_keyboard_actor_role:
                registered_session->raise_surface_to_layer(token, default_osk_layer);
                break;
            case ubuntu::application::ui::shutdown_dialog_actor_role:
                registered_session->raise_surface_to_layer(token, default_shutdown_dialog_layer);
                break;
        }
    } else
    {
        size_t i = 0;
        for(i = 0; i < apps_as_added.size(); i++)
        {
            if (apps_as_added[i] == session->asBinder())
                break;
        }

        switch_focused_application_locked(i);
    }
}

void ApplicationManager::request_update_for_session(const android::sp<android::IApplicationManagerSession>& session)
{
    ALOGI("%s", __PRETTY_FUNCTION__);
    android::Mutex::Autolock al(guard);

    if (apps_as_added[focused_application] != session->asBinder())
        return;

    const android::sp<mir::ApplicationSession>& as =
            apps.valueFor(apps_as_added[focused_application]);

    if (as->session_type == ubuntu::application::ui::system_session_type)
    {
        input_setup->input_manager->getDispatcher()->setFocusedApplication(
            shell_input_setup->shell_application);
        android::Vector< android::sp<android::InputWindowHandle> > input_windows;
        input_windows.push(shell_input_setup->event_trap_window.input_window);
        input_setup->input_manager->getDispatcher()->setInputWindows(
            input_windows);
    } else
    {
        input_setup->input_manager->getDispatcher()->setFocusedApplication(
            as->input_application_handle());

        android::Vector< android::sp<android::InputWindowHandle> > input_windows;

        if (is_osk_visible)
            input_windows.push(shell_input_setup->osk_window.input_window);
        if (are_notifications_visible)
            input_windows.push(shell_input_setup->notifications_window.input_window);

        input_windows.appendVector(as->input_window_handles());
        input_setup->input_manager->getDispatcher()->setInputWindows(
            input_windows);
    }
}

void ApplicationManager::register_an_observer(
    const android::sp<android::IApplicationManagerObserver>& observer)
{
    android::Mutex::Autolock al(observer_guard);
    app_manager_observers.push_back(observer);
    {
        android::Mutex::Autolock al(guard);

        for(unsigned int i = 0; i < apps_as_added.size(); i++)
        {
            const android::sp<mir::ApplicationSession>& session =
                    apps.valueFor(apps_as_added[i]);

            observer->on_session_born(session->remote_pid,
                                      session->desktop_file);
        }

        if (focused_application < apps_as_added.size())
        {
            const android::sp<mir::ApplicationSession>& session =
                    apps.valueFor(apps_as_added[focused_application]);

            observer->on_session_focused(session->remote_pid,
                                         session->desktop_file);
        }
    }
}

void ApplicationManager::focus_running_session_with_id(int id)
{
    android::Mutex::Autolock al(guard);

    size_t idx = session_id_to_index(id);

    if (idx < apps_as_added.size())
    {
        switch_focused_application_locked(idx);
    }
}

void ApplicationManager::unfocus_running_sessions()
{
    ALOGI("%s", __PRETTY_FUNCTION__);
    
    android::Mutex::Autolock al(guard);

    input_setup->input_manager->getDispatcher()->setFocusedApplication(
        shell_input_setup->shell_application);
    android::Vector< android::sp<android::InputWindowHandle> > input_windows;
        input_windows.push(shell_input_setup->event_trap_window.input_window);
    input_setup->input_manager->getDispatcher()->setInputWindows(
        input_windows);

    if (focused_application < apps.size())
    {
        const android::sp<mir::ApplicationSession>& session =
                apps.valueFor(apps_as_added[focused_application]);
        
        if (session->session_type != ubuntu::application::ui::system_session_type)
        {            
            notify_observers_about_session_unfocused(session->remote_pid,
                                                     session->desktop_file);

            // Stop the session
            if (!is_session_allowed_to_run_in_background(session))
            {
                ALOGI("\t Trying to stop ordinary app process.");
                if (0 != kill(session->remote_pid, SIGSTOP))
                {
                    ALOGI("\t Problem stopping process, errno = %d.", errno);
                } else
                {
                    ALOGI("\t\t Successfully stopped process.");
                }
            }
        }
    }
    shell_input_setup->shell_has_focus = true;
}

int32_t ApplicationManager::query_snapshot_layer_for_session_with_id(int id)
{
    size_t idx = session_id_to_index(id);

    if (idx < apps_as_added.size())
    {
        return apps.valueFor(apps_as_added[idx])->layer();
    }

    return INT_MAX;
}

void ApplicationManager::switch_to_well_known_application(int32_t app)
{
    notify_observers_about_session_requested(app);
}

void ApplicationManager::report_osk_visible(int32_t x, int32_t y, int32_t width, int32_t height)
{
    ALOGI("%s(x=%d, y=%d, width=%d, height=%d)", __PRETTY_FUNCTION__, x, y, width, height);
    
    shell_input_setup->osk_window.input_window->x = x;
    shell_input_setup->osk_window.input_window->y = y;   
    shell_input_setup->osk_window.input_window->w = width;
    shell_input_setup->osk_window.input_window->h = height;

    android::Mutex::Autolock al(guard);
    is_osk_visible = true;

    update_input_setup_locked();
}

void ApplicationManager::report_osk_invisible()
{
    ALOGI("%s", __PRETTY_FUNCTION__);
    android::Mutex::Autolock al(guard);
    is_osk_visible = false;

    update_input_setup_locked();
}

void ApplicationManager::report_notification_visible()
{
    ALOGI("%s", __PRETTY_FUNCTION__);
    android::Mutex::Autolock al(guard);
    are_notifications_visible = true;

    update_input_setup_locked();
}

void ApplicationManager::report_notification_invisible()
{
    ALOGI("%s", __PRETTY_FUNCTION__);
    android::Mutex::Autolock al(guard);
    are_notifications_visible = false;

    update_input_setup_locked();
}

void ApplicationManager::update_input_setup_locked()
{
    if (focused_application >= apps.size())
        return;
    
    const android::sp<mir::ApplicationSession>& session =
            apps.valueFor(apps_as_added[focused_application]);
    
    if (shell_input_setup->shell_has_focus)
    {
        input_setup->input_manager->getDispatcher()->setFocusedApplication(
            shell_input_setup->shell_application);
        
        android::Vector< android::sp<android::InputWindowHandle> > input_windows;
        input_windows.push(shell_input_setup->event_trap_window.input_window);

        input_setup->input_manager->getDispatcher()->setInputWindows(
            input_windows);
    } else
    {
        ALOGI("Adjusting input setup to account for change in visibility of osk/notifications");

        input_setup->input_manager->getDispatcher()->setFocusedApplication(
            session->input_application_handle());
        
        android::Vector< android::sp<android::InputWindowHandle> > input_windows;
        if (is_osk_visible)
            input_windows.push(shell_input_setup->osk_window.input_window);
        if (are_notifications_visible)
            input_windows.push(shell_input_setup->notifications_window.input_window);
        input_windows.appendVector(session->input_window_handles());
        input_setup->input_manager->getDispatcher()->setInputWindows(
            input_windows);
    }
}

void ApplicationManager::switch_focused_application_locked(size_t index_of_next_focused_app)
{
    static int focused_layer = 0;
    static const int focused_layer_increment = 10;

    if (apps.size() > 1 &&
        focused_application < apps.size() &&
        focused_application != index_of_next_focused_app)
    {
        //printf("\tLowering current application now for idx: %d \n", focused_application);
        const android::sp<mir::ApplicationSession>& session =
                apps.valueFor(apps_as_added[focused_application]);
        
        if (session->session_type != ubuntu::application::ui::system_session_type)
        {
            notify_observers_about_session_unfocused(session->remote_pid,
                                                     session->desktop_file);
            // Stop the session
            if (!is_session_allowed_to_run_in_background(session))
                kill(session->remote_pid, SIGSTOP);
        }
    }

    focused_application = index_of_next_focused_app;

    if (focused_application < apps.size())
    {
        focused_layer += focused_layer_increment;

        ALOGI("Raising application now for idx: %d \n", focused_application);
        const android::sp<mir::ApplicationSession>& session =
                apps.valueFor(apps_as_added[focused_application]);

        if (session->session_type == ubuntu::application::ui::system_session_type)
        {
            ALOGI("\t system session - not raising it.");
            return;
        }

        // Continue the session
        if (!is_session_allowed_to_run_in_background(session))
            kill(session->remote_pid, SIGCONT);

        session->raise_application_surfaces_to_layer(focused_layer);
        input_setup->input_manager->getDispatcher()->setFocusedApplication(
            session->input_application_handle());

        android::Vector< android::sp<android::InputWindowHandle> > input_windows;

        if (is_osk_visible)
            input_windows.push(shell_input_setup->osk_window.input_window);
        if (are_notifications_visible)
            input_windows.push(shell_input_setup->notifications_window.input_window);

        input_windows.appendVector(session->input_window_handles());
        input_setup->input_manager->getDispatcher()->setInputWindows(
            input_windows);

        notify_observers_about_session_focused(session->remote_pid,
                                               session->desktop_file);

        shell_input_setup->shell_has_focus = false;
    }
}

void ApplicationManager::switch_focus_to_next_application_locked()
{
    size_t new_idx = (focused_application + 1) % apps.size();

    ALOGI("current: %d, next: %d \n", focused_application, new_idx);

    switch_focused_application_locked(new_idx);
}

void ApplicationManager::kill_focused_application_locked()
{
    if (focused_application < apps.size())
    {
        const android::sp<mir::ApplicationSession>& session =
                apps.valueFor(apps_as_added[focused_application]);

        kill(session->remote_pid, SIGKILL);
    }
}

size_t ApplicationManager::session_id_to_index(int id)
{
    size_t idx = 0;

    for(idx = 0; idx < apps_as_added.size(); idx++)
    {
        const android::sp<mir::ApplicationSession>& session =
                apps.valueFor(apps_as_added[idx]);

        if (session->remote_pid == id)
            break;
    }

    return idx;
}

void ApplicationManager::notify_observers_about_session_requested(uint32_t app)
{
    android::Mutex::Autolock al(observer_guard);
    for(unsigned int i = 0; i < app_manager_observers.size(); i++)
    {
        app_manager_observers[i]->on_session_requested(app);
    }
}

void ApplicationManager::notify_observers_about_session_born(int id, const android::String8& desktop_file)
{
    android::Mutex::Autolock al(observer_guard);
    for(unsigned int i = 0; i < app_manager_observers.size(); i++)
    {
        app_manager_observers[i]->on_session_born(id, desktop_file);
    }
}

void ApplicationManager::notify_observers_about_session_unfocused(int id, const android::String8& desktop_file)
{
    android::Mutex::Autolock al(observer_guard);
    for(unsigned int i = 0; i < app_manager_observers.size(); i++)
    {
        app_manager_observers[i]->on_session_unfocused(id, desktop_file);
    }
}

void ApplicationManager::notify_observers_about_session_focused(int id, const android::String8& desktop_file)
{
    android::Mutex::Autolock al(observer_guard);
    for(unsigned int i = 0; i < app_manager_observers.size(); i++)
    {
        app_manager_observers[i]->on_session_focused(id, desktop_file);
    }
}

void ApplicationManager::notify_observers_about_session_died(int id, const android::String8& desktop_file)
{
    android::Mutex::Autolock al(observer_guard);
    for(unsigned int i = 0; i < app_manager_observers.size(); i++)
    {
        app_manager_observers[i]->on_session_died(id, desktop_file);
    }
}

}

int main(int argc, char** argv)
{
    android::sp<mir::ApplicationManager> app_manager(new mir::ApplicationManager());

    // Register service
    android::sp<android::IServiceManager> service_manager = android::defaultServiceManager();
    if (android::NO_ERROR != service_manager->addService(
            android::String16(android::IApplicationManager::exported_service_name()),
            app_manager))
    {
        //printf("Error registering service with the system ... exiting now.");
        return EXIT_FAILURE;
    }

    android::ProcessState::self()->startThreadPool();
    android::IPCThreadState::self()->joinThreadPool();
}
