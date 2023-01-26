/*
 * Copyright (c) 2020-2023 RaccoonLab.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#ifndef SRC_SENSORS_GNSS_HPP
#define SRC_SENSORS_GNSS_HPP

#include "sensor_base.hpp"
#include <Eigen/Geometry>

class GpsSensor : public BaseSensor{
    public:
        GpsSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(const Eigen::Vector3d& gpsPosition);
};

#endif  // SRC_SENSORS_GNSS_HPP
