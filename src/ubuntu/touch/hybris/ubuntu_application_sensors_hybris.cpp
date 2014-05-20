/*
 * Copyright (C) 2012 Canonical Ltd
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
 *              Ricardo Mendoza <ricardo.mendoza@canonical.com>
 */

#include <ubuntu/application/sensors/accelerometer.h>
#include <ubuntu/application/sensors/proximity.h>
#include <ubuntu/application/sensors/light.h>

#include "hybris_module.h"

// Ubuntu Application Sensors

// Acceleration Sensor
IMPLEMENT_CTOR0(UASensorsAccelerometer*, ua_sensors_accelerometer_new);
IMPLEMENT_FUNCTION1(UStatus, ua_sensors_accelerometer_enable, UASensorsAccelerometer*);
IMPLEMENT_FUNCTION1(UStatus, ua_sensors_accelerometer_disable, UASensorsAccelerometer*);
IMPLEMENT_FUNCTION1(uint32_t, ua_sensors_accelerometer_get_min_delay, UASensorsAccelerometer*);
IMPLEMENT_FUNCTION2(UStatus, ua_sensors_accelerometer_get_min_value, UASensorsAccelerometer*, float*);
IMPLEMENT_FUNCTION2(UStatus, ua_sensors_accelerometer_get_max_value, UASensorsAccelerometer*, float*);
IMPLEMENT_FUNCTION2(UStatus, ua_sensors_accelerometer_get_resolution, UASensorsAccelerometer*, float*);
IMPLEMENT_VOID_FUNCTION3(ua_sensors_accelerometer_set_reading_cb, UASensorsAccelerometer*, on_accelerometer_event_cb, void*);

// Acceleration Sensor Event
IMPLEMENT_FUNCTION1(uint64_t, uas_accelerometer_event_get_timestamp, UASAccelerometerEvent*);
IMPLEMENT_FUNCTION2(UStatus, uas_accelerometer_event_get_acceleration_x, UASAccelerometerEvent*, float*);
IMPLEMENT_FUNCTION2(UStatus, uas_accelerometer_event_get_acceleration_y, UASAccelerometerEvent*, float*);
IMPLEMENT_FUNCTION2(UStatus, uas_accelerometer_event_get_acceleration_z, UASAccelerometerEvent*, float*);

// Proximity Sensor
IMPLEMENT_CTOR0(UASensorsProximity*, ua_sensors_proximity_new);
IMPLEMENT_FUNCTION1(UStatus, ua_sensors_proximity_enable, UASensorsProximity*);
IMPLEMENT_FUNCTION1(UStatus, ua_sensors_proximity_disable, UASensorsProximity*);
IMPLEMENT_FUNCTION1(uint32_t, ua_sensors_proximity_get_min_delay, UASensorsProximity*);
IMPLEMENT_FUNCTION2(UStatus, ua_sensors_proximity_get_min_value, UASensorsProximity*, float*);
IMPLEMENT_FUNCTION2(UStatus, ua_sensors_proximity_get_max_value, UASensorsProximity*, float*);
IMPLEMENT_FUNCTION2(UStatus, ua_sensors_proximity_get_resolution, UASensorsProximity*, float*);
IMPLEMENT_VOID_FUNCTION3(ua_sensors_proximity_set_reading_cb, UASensorsProximity*, on_proximity_event_cb, void*);

// Proximity Sensor Event
IMPLEMENT_FUNCTION1(uint64_t, uas_proximity_event_get_timestamp, UASProximityEvent*);
IMPLEMENT_FUNCTION1(UASProximityDistance, uas_proximity_event_get_distance, UASProximityEvent*);

// Ambient Light Sensor
IMPLEMENT_CTOR0(UASensorsLight*, ua_sensors_light_new);
IMPLEMENT_FUNCTION1(UStatus, ua_sensors_light_enable, UASensorsLight*);
IMPLEMENT_FUNCTION1(UStatus, ua_sensors_light_disable, UASensorsLight*);
IMPLEMENT_FUNCTION1(uint32_t, ua_sensors_light_get_min_delay, UASensorsLight*);
IMPLEMENT_FUNCTION2(UStatus, ua_sensors_light_get_min_value, UASensorsLight*, float*);
IMPLEMENT_FUNCTION2(UStatus, ua_sensors_light_get_max_value, UASensorsLight*, float*);
IMPLEMENT_FUNCTION2(UStatus, ua_sensors_light_get_resolution, UASensorsLight*, float*);
IMPLEMENT_VOID_FUNCTION3(ua_sensors_light_set_reading_cb, UASensorsLight*, on_light_event_cb, void*);

// Ambient Light Sensor Event
IMPLEMENT_FUNCTION1(uint64_t, uas_light_event_get_timestamp, UASLightEvent*);
IMPLEMENT_FUNCTION2(UStatus, uas_light_event_get_light, UASLightEvent*, float*);
