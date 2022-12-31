/* 
 * Copyright (c) 2020-2022 RaccoonLab.
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

#include "sensors.hpp"
#include "sensors_isa_model.hpp"
#include "cs_converter.hpp"

Sensors::Sensors(ros::NodeHandle* nh) :
    attitudeSensor(nh,      "/uav/attitude",            0.005),
    imuSensor(nh,           "/uav/imu",                 0.00333),
    velocitySensor_(nh,     "/uav/velocity",            0.05),
    magSensor(nh,           "/uav/mag",                 0.03),
    rawAirDataSensor(nh,    "/uav/raw_air_data",        0.05),
    temperatureSensor(nh,   "/uav/static_temperature",  0.05),
    pressureSensor(nh,      "/uav/static_pressure",     0.05),
    gpsSensor(nh,           "/uav/gps_position",        0.1),
    escStatusSensor(nh,     "/uav/esc_status",          0.25),
    iceStatusSensor(nh,     "/uav/ice_status",          0.25),
    fuelTankSensor(nh,      "/uav/fuel_tank",           2.0),
    batteryInfoSensor(nh,   "/uav/battery",             1.0)
{
}

int8_t Sensors::init(UavDynamicsSimBase* uavDynamicsSim) {
    _uavDynamicsSim = uavDynamicsSim;

    double latRef, lonRef, altRef;
    const std::string SIM_PARAMS_PATH = "/uav/sim_params/";
    bool isEnabled;

    if(!ros::param::get(SIM_PARAMS_PATH + "lat_ref", latRef) ||
       !ros::param::get(SIM_PARAMS_PATH + "lon_ref", lonRef) ||
       !ros::param::get(SIM_PARAMS_PATH + "alt_ref", altRef)){
        ROS_ERROR("Sensors: lat_ref, lon_ref or alt_ref in not present.");
        return -1;
    }

    if (ros::param::get(SIM_PARAMS_PATH + "esc_status", isEnabled) && isEnabled) {
        escStatusSensor.enable();
    }

    if (ros::param::get(SIM_PARAMS_PATH + "ice_status", isEnabled) && isEnabled) {
        iceStatusSensor.enable();
    }

    if (ros::param::get(SIM_PARAMS_PATH + "fuel_tank_status", isEnabled) && isEnabled) {
        fuelTankSensor.enable();
    }

    if (ros::param::get(SIM_PARAMS_PATH + "battery_status", isEnabled) && isEnabled) {
        batteryInfoSensor.enable();
    }

    attitudeSensor.enable();
    imuSensor.enable();
    velocitySensor_.enable();
    magSensor.enable();
    rawAirDataSensor.enable();
    temperatureSensor.enable();
    pressureSensor.enable();
    gpsSensor.enable();

    geodeticConverter.initialiseReference(latRef, lonRef, altRef);

    return 0;
}

#define PX4_NED_FRD 0
#define ROS_ENU_FLU 1

/**
 * @note Different simulators return data in different notation (PX4 or ROS)
 * But we must publish only in PX4 notation
 */
void Sensors::publishStateToCommunicator(uint8_t dynamicsNotation) {
    // 1. Get data from simulator
    Eigen::Vector3d position, linVel, acc, gyro, angVel;
    Eigen::Quaterniond attitude;
    position = _uavDynamicsSim->getVehiclePosition();
    linVel = _uavDynamicsSim->getVehicleVelocity();
    _uavDynamicsSim->getIMUMeasurement(acc, gyro);
    angVel = _uavDynamicsSim->getVehicleAngularVelocity();
    attitude = _uavDynamicsSim->getVehicleAttitude();

    // 2. Convert them to appropriate CS
    Eigen::Vector3d gpsPosition, enuPosition, linVelNed, accFrd, gyroFrd, angVelFrd;
    Eigen::Quaterniond attitudeFrdToNed;
    if(dynamicsNotation == PX4_NED_FRD){
        enuPosition = Converter::nedToEnu(position);
        linVelNed = linVel;
        accFrd = acc;
        gyroFrd = gyro;
        angVelFrd = angVel;
        attitudeFrdToNed = attitude;
    }else{
        enuPosition = position;
        linVelNed =  Converter::enuToNed(linVel);
        accFrd = Converter::fluToFrd(acc);
        gyroFrd = Converter::fluToFrd(gyro);
        angVelFrd = Converter::fluToFrd(angVel);
        attitudeFrdToNed = Converter::fluEnuToFrdNed(attitude);
    }
    geodeticConverter.enu2Geodetic(enuPosition[0], enuPosition[1], enuPosition[2],
                                   &gpsPosition[0], &gpsPosition[1], &gpsPosition[2]);

    // 3. Calculate temperature, abs pressure and diff pressure using ISA model
    float temperatureKelvin, absPressureHpa, diffPressureHpa;
    SensorModelISA::EstimateAtmosphere(gpsPosition, linVelNed,
                                       temperatureKelvin, absPressureHpa, diffPressureHpa);

    // Publish state to communicator
    attitudeSensor.publish(attitudeFrdToNed);
    imuSensor.publish(accFrd, gyroFrd);
    velocitySensor_.publish(linVelNed, angVelFrd);
    magSensor.publish(gpsPosition, attitudeFrdToNed);
    rawAirDataSensor.publish(absPressureHpa, diffPressureHpa, temperatureKelvin);
    pressureSensor.publish(absPressureHpa);
    temperatureSensor.publish(temperatureKelvin);
    gpsSensor.publish(gpsPosition, linVelNed);

    std::vector<double> motorsRpm;
    if(_uavDynamicsSim->getMotorsRpm(motorsRpm)){
        escStatusSensor.publish(motorsRpm);
        if(motorsRpm.size() == 5){
            iceStatusSensor.publish(motorsRpm[4]);
        }
    }

    ///< @todo Simplified Fuel tank model, refactor it
    static double fuelLevelPercentage = 100.0;
    if(motorsRpm.size() == 5 && motorsRpm[4] >= 1) {
        fuelLevelPercentage -= 0.002;
        if(fuelLevelPercentage < 0) {
            fuelLevelPercentage = 0;
        }
    }
    fuelTankSensor.publish(fuelLevelPercentage);

    ///< @todo Battery is just constant, add model
    batteryInfoSensor.publish(90.0);
}