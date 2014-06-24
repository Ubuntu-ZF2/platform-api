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
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#include "ubuntu/application/location/session.h"

#include "session_p.h"

#include "heading_update_p.h"
#include "position_update_p.h"
#include "velocity_update_p.h"

namespace location = com::ubuntu::location;

void
ua_location_service_session_ref(
    UALocationServiceSession *session)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    s->ref();
}

void
ua_location_service_session_unref(
    UALocationServiceSession *session)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    s->unref();
}

void
ua_location_service_session_set_position_updates_handler(
    UALocationServiceSession *session,
    UALocationServiceSessionPositionUpdatesHandler handler,
    void *context)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    try
    {
        s->session->updates().position.changed().connect(
            [handler, context](const location::Update<location::Position>& new_position)
            {
                UbuntuApplicationLocationPositionUpdate pu{new_position};
                handler(std::addressof(pu), context);
            });
    } catch(const std::exception& e)
    {
        fprintf(stderr, "Error setting up position updates handler: %s \n", e.what());
    } catch(...)
    {
        fprintf(stderr, "Error setting up position updates handler.\n");
    }
}

void
ua_location_service_session_set_heading_updates_handler(
    UALocationServiceSession *session,
    UALocationServiceSessionHeadingUpdatesHandler handler,
    void *context)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    try
    {
        s->session->updates().heading.changed().connect(
                    [handler, context](const location::Update<location::Heading>& new_heading)
        {
            UbuntuApplicationLocationHeadingUpdate hu{new_heading};
            handler(std::addressof(hu), context);
        });
    } catch(const std::exception& e)
    {
        fprintf(stderr, "Error setting up heading updates handler: %s \n", e.what());
    } catch(...)
    {
        fprintf(stderr, "Error setting up heading updates handler. \n");
    }
}

void
ua_location_service_session_set_velocity_updates_handler(
    UALocationServiceSession *session,
    UALocationServiceSessionVelocityUpdatesHandler handler,
    void *context)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    try
    {
        s->session->updates().velocity.changed().connect(
                    [handler, context](const location::Update<location::Velocity>& new_velocity)
        {
            UbuntuApplicationLocationVelocityUpdate vu{new_velocity};
            handler(std::addressof(vu), context);
        });
    } catch(const std::exception& e)
    {
        fprintf(stderr, "Error setting up velocity updates handler: %s \n", e.what());
    } catch(...)
    {
        fprintf(stderr, "Error setting up velocity updates handler.");
    }
}

UStatus
ua_location_service_session_start_position_updates(
    UALocationServiceSession *session)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    if (!s)
        return U_STATUS_ERROR;

    try
    {
        s->session->updates().position_status.set(
                    location::service::session::Interface::Updates::Status::enabled);
    } catch(...)
    {
        return U_STATUS_ERROR;
    }
    
    return U_STATUS_SUCCESS;
}

void
ua_location_service_session_stop_position_updates(
    UALocationServiceSession *session)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    if (!s)
        return;

    try
    {
        s->session->updates().position_status.set(
                    location::service::session::Interface::Updates::Status::disabled);
    } catch(...)
    {
    }    
}

UStatus
ua_location_service_session_start_heading_updates(
    UALocationServiceSession *session)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    if (!s)
        return U_STATUS_ERROR;

    try
    {
        s->session->updates().heading_status.set(
                    location::service::session::Interface::Updates::Status::enabled);
    } catch(...)
    {
        return U_STATUS_ERROR;
    }
    
    return U_STATUS_SUCCESS;
}

void
ua_location_service_session_stop_heading_updates(
    UALocationServiceSession *session)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    if (!s)
        return;

    try
    {
        s->session->updates().heading_status.set(
                    location::service::session::Interface::Updates::Status::disabled);
    } catch(...)
    {
    }
}

UStatus
ua_location_service_session_start_velocity_updates(
    UALocationServiceSession *session)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    if (!s)
        return U_STATUS_ERROR;

    try
    {
        s->session->updates().velocity_status.set(
                    location::service::session::Interface::Updates::Status::enabled);
    } catch(...)
    {
        return U_STATUS_ERROR;
    }
    
    return U_STATUS_SUCCESS;
}

void
ua_location_service_session_stop_velocity_updates(
    UALocationServiceSession *session)
{
    auto s = static_cast<UbuntuApplicationLocationServiceSession*>(session);
    if (!s)
        return;

    try
    {
        s->session->updates().velocity_status.set(
                    location::service::session::Interface::Updates::Status::disabled);
    } catch(...)
    {
    }
}
