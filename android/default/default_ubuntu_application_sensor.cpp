/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 *              Ricardo Mendoza <ricardo.mendoza@canonical.com>
 */

#include <ubuntu/application/sensors/ubuntu_application_sensors.h>
#include <ubuntu/application/sensors/accelerometer.h>
#include <ubuntu/application/sensors/proximity.h>
#include <ubuntu/application/sensors/light.h>

#include <private/application/sensors/sensor.h>
#include <private/application/sensors/sensor_listener.h>
#include <private/application/sensors/sensor_service.h>
#include <private/application/sensors/sensor_type.h>
#include <private/application/sensors/events.h>

#include <cassert>
#include <cstdio>

namespace
{
template<typename T>
struct Holder
{
    Holder(const T&value = T()) : value(value)
    {
    }

    T value;
};

template<typename T>
Holder<T>* make_holder(const T& value)
{
    return new Holder<T>(value);
}

enum sensor_value_t { MIN_DELAY, MIN_VALUE, MAX_VALUE, RESOLUTION };
template<ubuntu::application::sensors::SensorType sensor_type>
struct SensorListener : public ubuntu::application::sensors::SensorListener
{
    SensorListener() : on_accelerometer_event(NULL),
                       on_proximity_event(NULL),
                       on_light_event(NULL)
    {
    }

    void on_new_reading(const ubuntu::application::sensors::SensorReading::Ptr& reading)
    {
        switch(sensor_type)
        {
            case ubuntu::application::sensors::sensor_type_accelerometer:
            {
                if (!on_accelerometer_event)
                    return;

                ubuntu::application::sensors::AccelerometerEvent::Ptr ev(
                    new ubuntu::application::sensors::AccelerometerEvent(
                        reading->timestamp,
                        reading->acceleration[0],
                        reading->acceleration[1],
                        reading->acceleration[2])
                        );

                on_accelerometer_event(
                    make_holder(ev), this->context
                    );

                break;
            }
            case ubuntu::application::sensors::sensor_type_proximity:
            {
                if (!on_proximity_event)
                    return;

                ubuntu::application::sensors::ProximityEvent::Ptr ev(
                    new ubuntu::application::sensors::ProximityEvent(
                        static_cast<uint64_t>(reading->timestamp),
                        reading->distance)
                    );

                on_proximity_event(
                    make_holder(ev), this->context
                    );

                break;
            }
            case ubuntu::application::sensors::sensor_type_light:
            {
                if (!on_light_event)
                    return;

                ubuntu::application::sensors::LightEvent::Ptr ev(
                    new ubuntu::application::sensors::LightEvent(
                        reading->timestamp,
                        reading->light)
                    );

                on_light_event(
                    make_holder(ev), this->context
                    );

                break;
            }
        }
    }

    on_accelerometer_event_cb on_accelerometer_event;
    on_proximity_event_cb on_proximity_event;
    on_light_event_cb on_light_event;
    void *context;
};

ubuntu::application::sensors::Sensor::Ptr accelerometer;
ubuntu::application::sensors::Sensor::Ptr proximity;
ubuntu::application::sensors::Sensor::Ptr light;
ubuntu::application::sensors::SensorListener::Ptr accelerometer_listener;
ubuntu::application::sensors::SensorListener::Ptr proximity_listener;
ubuntu::application::sensors::SensorListener::Ptr light_listener;
}

static int32_t toHz(int32_t microseconds)
{
    return 1 / (microseconds / 1e6);
}

/*
 * Proximity Sensor
 */

UASensorsProximity*
ua_sensors_proximity_new()
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);

    proximity =
        ubuntu::application::sensors::SensorService::sensor_for_type(
            ubuntu::application::sensors::sensor_type_proximity);

    return proximity.get();
}

UStatus
ua_sensors_proximity_enable(
    UASensorsProximity* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);

    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    int ret = s->enable();

    if (ret < 0)
        return U_STATUS_ERROR;

    return U_STATUS_SUCCESS;
}

UStatus
ua_sensors_proximity_disable(
    UASensorsProximity* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);

    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    s->disable();

    return U_STATUS_SUCCESS;
}

uint32_t
ua_sensors_proximity_get_min_delay(
    UASensorsProximity* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return toHz(s->min_delay());
}

float
ua_sensors_proximity_get_min_value(
    UASensorsProximity* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return s->min_value();
}

float
ua_sensors_proximity_get_max_value(
    UASensorsProximity* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return s->max_value();
}

float
ua_sensors_proximity_get_resolution(
    UASensorsProximity* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return s->resolution();
}

void
ua_sensors_proximity_set_reading_cb(
    UASensorsProximity* sensor,
    on_proximity_event_cb cb,
    void *ctx)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);

    SensorListener<ubuntu::application::sensors::sensor_type_proximity>* sl
        = new SensorListener<ubuntu::application::sensors::sensor_type_proximity>();

    sl->on_proximity_event = cb;
    sl->context = ctx;

    proximity_listener = sl;
    s->register_listener(proximity_listener);
}

uint64_t
uas_proximity_event_get_timestamp(
    UASProximityEvent* event)
{
    auto ev = static_cast<Holder<ubuntu::application::sensors::ProximityEvent::Ptr>*>(event);

    return ev->value->get_timestamp();
}

UASProximityDistance
uas_proximity_event_get_distance(
    UASProximityEvent* event)
{
    auto ev = static_cast<Holder<ubuntu::application::sensors::ProximityEvent::Ptr>*>(event);

    if (ev->value->get_distance() == proximity->max_value())
        return U_PROXIMITY_FAR;

    return U_PROXIMITY_NEAR;
}

/*
 * Ambient Light Sensor
 */

UASensorsLight*
ua_sensors_light_new()
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    light =
        ubuntu::application::sensors::SensorService::sensor_for_type(
            ubuntu::application::sensors::sensor_type_light);

    return light.get();
}

UStatus
ua_sensors_light_enable(
    UASensorsLight* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    
    s->enable();

    return U_STATUS_SUCCESS;
}

UStatus
ua_sensors_light_disable(
    UASensorsLight* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    s->disable();

    return U_STATUS_SUCCESS;
}

uint32_t
ua_sensors_light_get_min_delay(
    UASensorsLight* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return toHz(s->min_delay());
}

float
ua_sensors_light_get_min_value(
    UASensorsLight* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return s->min_value();
}

float
ua_sensors_light_get_max_value(
    UASensorsLight* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return s->max_value();
}

float
ua_sensors_light_get_resolution(
    UASensorsLight* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return s->resolution();
}

void
ua_sensors_light_set_reading_cb(
    UASensorsLight* sensor,
    on_light_event_cb cb,
    void *ctx)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);

    SensorListener<ubuntu::application::sensors::sensor_type_light>* sl
        = new SensorListener<ubuntu::application::sensors::sensor_type_light>();

    sl->on_light_event = cb;
    sl->context = ctx;

    light_listener = sl;
    s->register_listener(light_listener);
}

uint64_t
uas_light_event_get_timestamp(
    UASLightEvent* event)
{
    auto ev = static_cast<Holder<ubuntu::application::sensors::LightEvent::Ptr>*>(event);
    return ev->value->get_timestamp();
}

float
uas_light_event_get_light(
    UASLightEvent* event)
{
    auto ev = static_cast<Holder<ubuntu::application::sensors::LightEvent::Ptr>*>(event);
    return ev->value->get_light();
}

/*
 * Acceleration Sensor
 */

UASensorsAccelerometer*
ua_sensors_accelerometer_new()
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    accelerometer =
        ubuntu::application::sensors::SensorService::sensor_for_type(
            ubuntu::application::sensors::sensor_type_accelerometer);

    return accelerometer.get();
}

UStatus
ua_sensors_accelerometer_enable(
    UASensorsAccelerometer* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    
    s->enable();

    return U_STATUS_SUCCESS;
}

UStatus
ua_sensors_accelerometer_disable(
    UASensorsAccelerometer* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    s->disable();

    return U_STATUS_SUCCESS;
}

uint32_t
ua_sensors_accelerometer_get_min_delay(
    UASensorsAccelerometer* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return toHz(s->min_delay());
}

float
ua_sensors_accelerometer_get_min_value(
    UASensorsAccelerometer* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return s->min_value();
}

float
ua_sensors_accelerometer_get_max_value(
    UASensorsAccelerometer* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return s->max_value();
}

float
ua_sensors_accelerometer_get_resolution(
    UASensorsAccelerometer* sensor)
{
    ALOGI("%s():%d", __PRETTY_FUNCTION__, __LINE__);
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);
    return s->resolution();
}

void
ua_sensors_accelerometer_set_reading_cb(
    UASensorsAccelerometer* sensor,
    on_accelerometer_event_cb cb,
    void *ctx)
{
    auto s = static_cast<ubuntu::application::sensors::Sensor*>(sensor);

    SensorListener<ubuntu::application::sensors::sensor_type_accelerometer>* sl
        = new SensorListener<ubuntu::application::sensors::sensor_type_accelerometer>();

    sl->on_accelerometer_event = cb;
    sl->context = ctx;

    accelerometer_listener = sl;
    s->register_listener(accelerometer_listener);
}

uint64_t
uas_accelerometer_event_get_timestamp(
    UASAccelerometerEvent* event)
{
    auto ev = static_cast<Holder<ubuntu::application::sensors::AccelerometerEvent::Ptr>*>(event);
    return ev->value->get_timestamp();
}

float
uas_accelerometer_event_get_acceleration_x(
    UASAccelerometerEvent* event)
{
    auto ev = static_cast<Holder<ubuntu::application::sensors::AccelerometerEvent::Ptr>*>(event);
    return ev->value->get_x();
}

float
uas_accelerometer_event_get_acceleration_y(
    UASAccelerometerEvent* event)
{
    auto ev = static_cast<Holder<ubuntu::application::sensors::AccelerometerEvent::Ptr>*>(event);
    return ev->value->get_y();
}

float
uas_accelerometer_event_get_acceleration_z(
    UASAccelerometerEvent* event)
{
    auto ev = static_cast<Holder<ubuntu::application::sensors::AccelerometerEvent::Ptr>*>(event);
    return ev->value->get_z();
}
