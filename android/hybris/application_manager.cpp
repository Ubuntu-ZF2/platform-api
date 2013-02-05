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
#include "application_manager.h"

#include <binder/Parcel.h>
#include <utils/String8.h>

namespace android
{
IMPLEMENT_META_INTERFACE(ApplicationManagerObserver, "UbuntuApplicationManagerObserver");
IMPLEMENT_META_INTERFACE(ApplicationManagerSession, "UbuntuApplicationManagerSession");
IMPLEMENT_META_INTERFACE(ApplicationManager, "UbuntuApplicationManager");

BnApplicationManagerSession::BnApplicationManagerSession()
{
}

BnApplicationManagerSession::~BnApplicationManagerSession() {}

status_t BnApplicationManagerSession::onTransact(uint32_t code,
        const Parcel& data,
        Parcel* reply,
        uint32_t flags)
{
    switch(code)
    {
    case RAISE_APPLICATION_SURFACES_TO_LAYER_COMMAND:
    {
        int32_t layer;
        data.readInt32(&layer);

        raise_application_surfaces_to_layer(layer);
    }
    break;
    case RAISE_SURFACE_TO_LAYER_COMMAND:
    {
        int32_t token, layer;
        token = data.readInt32();
        layer = data.readInt32();
        
        raise_surface_to_layer(token, layer);
    }
    break;
    case QUERY_SURFACE_PROPERTIES_FOR_TOKEN_COMMAND:
    {
        int32_t token = data.readInt32();
        IApplicationManagerSession::SurfaceProperties props =
            query_surface_properties_for_token(token);
        reply->writeInt32(props.layer);
        reply->writeInt32(props.left);
        reply->writeInt32(props.top);
        reply->writeInt32(props.right);
        reply->writeInt32(props.bottom);
    }
    }
    return NO_ERROR;
}

BpApplicationManagerSession::BpApplicationManagerSession(const sp<IBinder>& impl)
    : BpInterface<IApplicationManagerSession>(impl)
{
}

BpApplicationManagerSession::~BpApplicationManagerSession()
{
}

void BpApplicationManagerSession::raise_surface_to_layer(int32_t token, int layer)
{
    Parcel in, out;

    in.writeInt32(token);
    in.writeInt32(layer);
    
    remote()->transact(
        RAISE_SURFACE_TO_LAYER_COMMAND,
        in,
        &out);
}

void BpApplicationManagerSession::raise_application_surfaces_to_layer(int layer)
{
    Parcel in, out;
    in.writeInt32(layer);

    remote()->transact(
        RAISE_APPLICATION_SURFACES_TO_LAYER_COMMAND,
        in,
        &out);
}

IApplicationManagerSession::SurfaceProperties BpApplicationManagerSession::query_surface_properties_for_token(int32_t token)
{
    Parcel in, out;
    in.writeInt32(token);

    remote()->transact(
        QUERY_SURFACE_PROPERTIES_FOR_TOKEN_COMMAND,
        in,
        &out);

    IApplicationManagerSession::SurfaceProperties props;
    props.layer = out.readInt32();
    props.left = out.readInt32();
    props.top = out.readInt32();
    props.right = out.readInt32();
    props.bottom = out.readInt32();

    return props;
}

status_t BnApplicationManagerObserver::onTransact(uint32_t code,
        const Parcel& data,
        Parcel* reply,
        uint32_t flags)
{
    

    switch(code)
    {
    case ON_SESSION_REQUESTED_NOTIFICATION:
        {
            uint32_t app = data.readInt32();
            on_session_requested(app);
            break;
        }
    case ON_SESSION_BORN_NOTIFICATION:
        {
            int id = data.readInt32();
            String8 desktop_file = data.readString8();
            on_session_born(id, desktop_file);
            break;
        }
    case ON_SESSION_UNFOCUSED_NOTIFICATION:
        {
            int id = data.readInt32();
            String8 desktop_file = data.readString8();
            on_session_unfocused(id, desktop_file);
            break;
        }
    case ON_SESSION_FOCUSED_NOTIFICATION:
        {
            int id = data.readInt32();
            String8 desktop_file = data.readString8();
            on_session_focused(id, desktop_file);
            break;
        }
    case ON_SESSION_DIED_NOTIFICATION:
        {
            int id = data.readInt32();
            String8 desktop_file = data.readString8();
            on_session_died(id, desktop_file);
            break;
        }
    }

    return NO_ERROR;
}

BpApplicationManagerObserver::BpApplicationManagerObserver(const sp<IBinder>& impl)
    : BpInterface<IApplicationManagerObserver>(impl)
{
}

void BpApplicationManagerObserver::on_session_requested(
    uint32_t app)
{
    Parcel in, out;
    in.writeInt32(app);

    remote()->transact(
        ON_SESSION_REQUESTED_NOTIFICATION,
        in,
        &out,
        android::IBinder::FLAG_ONEWAY);
}

void BpApplicationManagerObserver::on_session_born(int id,
        const String8& desktop_file_hint)
{
    Parcel in, out;
    in.writeInt32(id);
    in.writeString8(desktop_file_hint);

    remote()->transact(
        ON_SESSION_BORN_NOTIFICATION,
        in,
        &out,
        android::IBinder::FLAG_ONEWAY);
}

void BpApplicationManagerObserver::on_session_unfocused(int id,
        const String8& desktop_file_hint)
{
    Parcel in, out;
    in.writeInt32(id);
    in.writeString8(desktop_file_hint);

    remote()->transact(
        ON_SESSION_UNFOCUSED_NOTIFICATION,
        in,
        &out,
        android::IBinder::FLAG_ONEWAY);
}

void BpApplicationManagerObserver::on_session_focused(int id,
        const String8& desktop_file_hint)
{
    Parcel in, out;
    in.writeInt32(id);
    in.writeString8(desktop_file_hint);

    remote()->transact(
        ON_SESSION_FOCUSED_NOTIFICATION,
        in,
        &out,
        android::IBinder::FLAG_ONEWAY);
}

void BpApplicationManagerObserver::on_session_died(int id,
        const String8& desktop_file_hint)
{
    Parcel in, out;
    in.writeInt32(id);
    in.writeString8(desktop_file_hint);

    remote()->transact(
        ON_SESSION_DIED_NOTIFICATION,
        in,
        &out,
        android::IBinder::FLAG_ONEWAY);
}

BnApplicationManager::BnApplicationManager()
{
}

BnApplicationManager::~BnApplicationManager()
{
}

status_t BnApplicationManager::onTransact(uint32_t code,
        const Parcel& data,
        Parcel* reply,
        uint32_t flags)
{
    switch(code)
    {
    case START_A_NEW_SESSION_COMMAND:
    {
        int32_t session_type = data.readInt32();
        String8 app_name = data.readString8();
        String8 desktop_file = data.readString8();
        sp<IBinder> binder = data.readStrongBinder();
        sp<BpApplicationManagerSession> session(new BpApplicationManagerSession(binder));
        int fd = data.readFileDescriptor();

        start_a_new_session(session_type, app_name, desktop_file, session, fd);
    }
    break;
    case REGISTER_A_SURFACE_COMMAND:
    {
        String8 title = data.readString8();
        sp<IBinder> binder = data.readStrongBinder();
        sp<BpApplicationManagerSession> session(new BpApplicationManagerSession(binder));
        int32_t surface_role = data.readInt32();
        int32_t surface_token = data.readInt32();
        int fd = data.readFileDescriptor();

        register_a_surface(title, session, surface_role, surface_token, fd);
    }
    break;
    case REGISTER_AN_OBSERVER_COMMAND:
    {
        sp<IBinder> binder = data.readStrongBinder();
        sp<BpApplicationManagerObserver> observer(new BpApplicationManagerObserver(binder));
        register_an_observer(observer);
        break;
    }
    case REQUEST_UPDATE_FOR_SESSION_COMMAND:
    {
        sp<IBinder> binder = data.readStrongBinder();
        sp<BpApplicationManagerSession> session(new BpApplicationManagerSession(binder));
        request_update_for_session(session);
        break;
    }
    case UNFOCUS_RUNNING_SESSIONS_COMMAND:
    {
        unfocus_running_sessions();
        break;
    }
    case FOCUS_RUNNING_SESSION_WITH_ID_COMMAND:
    {
        int32_t id = data.readInt32();
        focus_running_session_with_id(id);
        break;
    }
    case QUERY_SNAPSHOT_LAYER_FOR_SESSION_WITH_ID_COMMAND:
    {
        int32_t id = data.readInt32();
        int32_t layer = query_snapshot_layer_for_session_with_id(id);
        reply->writeInt32(layer);
        break;
    }        
    case SWITCH_TO_WELL_KNOWN_APPLICATION_COMMAND:
    {
        int32_t app = data.readInt32();
        switch_to_well_known_application(app);
        break;
    }
    case REPORT_OSK_VISIBLE_COMMAND:
    {
        int32_t x = data.readInt32();
        int32_t y = data.readInt32();
        int32_t width = data.readInt32();
        int32_t height = data.readInt32();
        report_osk_visible(x, y, width, height);
        break;
    }
    case REPORT_OSK_INVISIBLE_COMMAND:
    {
        report_osk_invisible();
        break;
    }
    case REPORT_NOTIFICATION_VISIBLE_COMMAND:
    {
        report_notification_visible();
        break;
    }
    case REPORT_NOTIFICATION_INVISIBLE_COMMAND:
    {
        report_notification_invisible();
        break;
    }
    }
    return NO_ERROR;
}

BpApplicationManager::BpApplicationManager(const sp<IBinder>& impl)
    : BpInterface<IApplicationManager>(impl)
{
}

BpApplicationManager::~BpApplicationManager()
{
}

void BpApplicationManager::start_a_new_session(
    int32_t session_type,
    const String8& app_name,
    const String8& desktop_file,
    const sp<IApplicationManagerSession>& session,
    int fd)
{
    //printf("%s \n", __PRETTY_FUNCTION__);
    Parcel in, out;
    in.pushAllowFds(true);
    in.writeInt32(session_type);
    in.writeString8(app_name);
    in.writeString8(desktop_file);
    in.writeStrongBinder(session->asBinder());
    in.writeFileDescriptor(fd);

    remote()->transact(START_A_NEW_SESSION_COMMAND,
                       in,
                       &out);
}

void BpApplicationManager::register_a_surface(
    const String8& title,
    const sp<IApplicationManagerSession>& session,
    int32_t surface_role,
    int32_t token,
    int fd)
{
    //printf("%s \n", __PRETTY_FUNCTION__);
    Parcel in, out;
    in.pushAllowFds(true);
    in.writeString8(title);
    in.writeStrongBinder(session->asBinder());
    in.writeInt32(surface_role);
    in.writeInt32(token);
    in.writeFileDescriptor(fd);

    remote()->transact(REGISTER_A_SURFACE_COMMAND,
                       in,
                       &out);
}

void BpApplicationManager::register_an_observer(const sp<IApplicationManagerObserver>& observer)
{
    Parcel in, out;
    in.writeStrongBinder(observer->asBinder());

    remote()->transact(REGISTER_AN_OBSERVER_COMMAND,
                       in,
                       &out);
}

void BpApplicationManager::request_update_for_session(const sp<IApplicationManagerSession>& session)
{
    Parcel in, out;
    in.writeStrongBinder(session->asBinder());
    remote()->transact(REQUEST_UPDATE_FOR_SESSION_COMMAND,
                       in,
                       &out);
}

void BpApplicationManager::unfocus_running_sessions()
{
    Parcel in, out;

    remote()->transact(UNFOCUS_RUNNING_SESSIONS_COMMAND,
                       in,
                       &out);
}

void BpApplicationManager::focus_running_session_with_id(int id)
{
    Parcel in, out;
    in.writeInt32(id);

    remote()->transact(FOCUS_RUNNING_SESSION_WITH_ID_COMMAND,
                       in,
                       &out);
}

int32_t BpApplicationManager::query_snapshot_layer_for_session_with_id(int id)
{
    Parcel in, out;
    in.writeInt32(id);
    remote()->transact(QUERY_SNAPSHOT_LAYER_FOR_SESSION_WITH_ID_COMMAND,
                       in,
                       &out);

    int32_t layer = out.readInt32();
    return layer;
}

void BpApplicationManager::switch_to_well_known_application(int32_t app)
{
    Parcel in, out;
    in.writeInt32(app);

    remote()->transact(SWITCH_TO_WELL_KNOWN_APPLICATION_COMMAND,
                       in,
                       &out);
}

void BpApplicationManager::report_osk_visible(int32_t x, int32_t y, int32_t width, int32_t height)
{
    Parcel in, out;
    in.writeInt32(x);
    in.writeInt32(y);
    in.writeInt32(width);
    in.writeInt32(height);

    remote()->transact(REPORT_OSK_VISIBLE_COMMAND,
                       in,
                       &out);
}
    
void BpApplicationManager::report_osk_invisible()
{
    Parcel in, out;

    remote()->transact(REPORT_OSK_INVISIBLE_COMMAND,
                       in,
                       &out);
}
    
void BpApplicationManager::report_notification_visible()
{
    Parcel in, out;

    remote()->transact(REPORT_NOTIFICATION_VISIBLE_COMMAND,
                       in,
                       &out);
}
    
void BpApplicationManager::report_notification_invisible()
{
    Parcel in, out;

    remote()->transact(REPORT_NOTIFICATION_INVISIBLE_COMMAND,
                       in,
                       &out);
}

}
