/**
 * @file sensors.hpp
 * @author Dmitry Ponomarev
 */

#ifndef INNO_VTOL_DYNAMICS_SENSORS_HPP
#define INNO_VTOL_DYNAMICS_SENSORS_HPP

#include <iostream>
#include <ros/ros.h>
#include <ros/time.h>
#include <Eigen/Geometry>
#include <random>
#include <geographiclib_conversions/geodetic_conv.hpp>
#include <uavcan_msgs/IceReciprocatingStatus.h>

class BaseSensor{
    public:
        BaseSensor() = delete;
        BaseSensor(ros::NodeHandle* nh, double period): node_handler_(nh), PERIOD(period) {};
        void enable() {isEnabled_ = true;}
        void disable() {isEnabled_ = false;}
    protected:
        ros::NodeHandle* node_handler_;
        bool isEnabled_{false};
        const double PERIOD;
        ros::Publisher publisher_;
        double nextPubTimeSec_ = 0;

        std::default_random_engine randomGenerator_;
        std::normal_distribution<double> normalDistribution_{std::normal_distribution<double>(0.0, 1.0)};
};

class AttitudeSensor : public BaseSensor{
    public:
        AttitudeSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(const Eigen::Quaterniond& attitudeFrdToNed);
};

class BatteryInfoSensor : public BaseSensor{
    public:
        BatteryInfoSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(double rpm);
};

class EscStatusSensor : public BaseSensor{
    public:
        EscStatusSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(const std::vector<double>& rpm);
    private:
        uint8_t nextEscIdx_ = 0;
};

class FuelTankSensor : public BaseSensor{
    public:
        FuelTankSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(double rpm);
};

class GpsSensor : public BaseSensor{
    public:
        GpsSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(const Eigen::Vector3d& gpsPosition, const Eigen::Vector3d& nedVelocity);
    private:
        ros::Publisher yaw_publisher_;
        ros::Publisher position_publisher_;
        ros::Publisher velocity_publisher_;
};

class IceStatusSensor : public BaseSensor{
    public:
        IceStatusSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(double rpm);
        void start_stall_emulation();
    private:
        void estimate_state(double rpm);
        void emulate_normal_mode(double rpm);
        void emulate_stall_mode();
        uavcan_msgs::IceReciprocatingStatus _iceStatusMsg;
        double _stallTsMs = 0;
        uint32_t _startTsSec = 0;
};

class ImuSensor : public BaseSensor{
    public:
        ImuSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(const Eigen::Vector3d& accFrd, const Eigen::Vector3d& gyroFrd);
};

class MagSensor : public BaseSensor{
    public:
        MagSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(const Eigen::Vector3d& geoPosition, const Eigen::Quaterniond& attitudeFrdToNed);
};

class RawAirDataSensor : public BaseSensor{
    public:
        RawAirDataSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(float absPressureHpa, float diffPressure, float staticTemperature);
};

class PressureSensor : public BaseSensor{
    public:
        PressureSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(float staticPressureHpa);
    private:
        ros::Publisher old_publisher_;
};

class TemperatureSensor : public BaseSensor{
    public:
        TemperatureSensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(float staticTemperature);
    private:
        ros::Publisher old_publisher_;
};

class VelocitySensor : public BaseSensor{
    public:
        VelocitySensor(ros::NodeHandle* nh, const char* topic, double period);
        bool publish(const Eigen::Vector3d& linVelNed, const Eigen::Vector3d& angVelFrd);
};


#endif  // INNO_VTOL_DYNAMICS_SENSORS_HPP
