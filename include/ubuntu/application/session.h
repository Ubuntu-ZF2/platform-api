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
#ifndef UBUNTU_APPLICATION_SESSION_H_
#define UBUNTU_APPLICATION_SESSION_H_

#include "ubuntu/platform/shared_ptr.h"

namespace ubuntu
{
namespace application
{
class Session : public ubuntu::platform::ReferenceCountedBase
{
public:
    typedef ubuntu::platform::shared_ptr<Session> Ptr;

protected:
    Session() {}
    virtual ~Session() {}

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
};
}
}

#endif // UBUNTU_APPLICATION_SESSION_H_
