/**
 * @file uavDynamicsNode.cpp
 * @author Dmitry Ponomarev
 * @author Roman Fedorenko
 * @author Ezra Tal
 * @author Winter Guerra
 * @author Varun Murali
 * @brief Implementation of UAV dynamics, IMU, and angular rate control simulation node
 */

#include "uavDynamicsNode.hpp"
#include <rosgraph_msgs/Clock.h>
#include <geometry_msgs/TransformStamped.h>
#include <std_msgs/Time.h>

#include "flightgogglesDynamicsSim.hpp"
#include "vtolDynamicsSim.hpp"
#include "cs_converter.hpp"


int main(int argc, char **argv){
    ros::init(argc, argv, "uav_dynamics_node");
    if( ros::console::set_logger_level(ROSCONSOLE_DEFAULT_NAME, ros::console::levels::Info) ) {
        ros::console::notifyLoggerLevelsChanged();
    }

    ros::NodeHandle node_handler("inno_dynamics_sim");
    Uav_Dynamics uav_dynamics_node(node_handler);
    if(uav_dynamics_node.init() == -1){
        ROS_ERROR("Shutdown.");
        ros::shutdown();
        return -1;
    }

    ros::spin();
    return 0;
}


Uav_Dynamics::Uav_Dynamics(ros::NodeHandle nh) :
    node_(nh),
    actuators_(8, 0.),
    initPose_(7),
    _sensors(&nh),
    _rviz_visualizator(node_){
}


/**
 * @return -1 if error occured, else 0
 */
int8_t Uav_Dynamics::init(){
    if(getParamsFromRos() == -1){
        return -1;
    }else if(initDynamicsSimulator() == -1){
        return -1;
    }else if(initSensors() == -1){
        return -1;
    }else if(initCalibration() == -1){
        return -1;
    }else if(_rviz_visualizator.init(uavDynamicsSim_) == -1){
        return -1;
    }else if(startClockAndThreads() == -1){
        return -1;
    }

    return 0;
}

int8_t Uav_Dynamics::getParamsFromRos(){
    const std::string SIM_PARAMS_PATH = "/uav/sim_params/";
    if(!ros::param::get(SIM_PARAMS_PATH + "use_sim_time",       useSimTime_ )           ||
       !node_.getParam("vehicle",                               vehicleName_)           ||
       !node_.getParam("dynamics",                              dynamicsTypeName_)      ||
       !ros::param::get(SIM_PARAMS_PATH + "init_pose",          initPose_)){
        ROS_ERROR("Dynamics: There is no at least one of required simulator parameters.");
        return -1;
    }
    return 0;
}

int8_t Uav_Dynamics::initDynamicsSimulator(){
    const char DYNAMICS_NAME_FLIGHTGOGGLES[] = "flightgoggles_multicopter";
    const char DYNAMICS_NAME_INNO_VTOL[] = "inno_vtol";
    const char VEHICLE_NAME_INNOPOLIS_VTOL[] = "innopolis_vtol";
    const char VEHICLE_NAME_IRIS[] = "iris";
    if(dynamicsTypeName_ == DYNAMICS_NAME_FLIGHTGOGGLES){
        dynamicsType_ = DYNAMICS_FLIGHTGOGGLES_MULTICOPTER;
        uavDynamicsSim_ = new FlightgogglesDynamics;
        _dynamicsNotation = ROS_ENU_FLU;
    }else if(dynamicsTypeName_ == DYNAMICS_NAME_INNO_VTOL){
        uavDynamicsSim_ = new InnoVtolDynamicsSim;
        dynamicsType_ = DYNAMICS_INNO_VTOL;
        _dynamicsNotation = PX4_NED_FRD;
    }else{
        ROS_ERROR("Dynamics type with name \"%s\" is not exist.", dynamicsTypeName_.c_str());
        return -1;
    }

    if(vehicleName_ == VEHICLE_NAME_INNOPOLIS_VTOL){
        vehicleType_ = VEHICLE_INNOPOLIS_VTOL;
    }else if(vehicleName_ == VEHICLE_NAME_IRIS){
        vehicleType_ = VEHICLE_IRIS;
    }else{
        ROS_ERROR("Wrong vehicle. It should be 'innopolis_vtol' or 'iris'");
        return -1;
    }

    if(uavDynamicsSim_ == nullptr || uavDynamicsSim_->init() == -1){
        ROS_ERROR("Can't init uav dynamics sim. Shutdown.");
        return -1;
    }

    Eigen::Vector3d initPosition(initPose_.at(0), initPose_.at(1), initPose_.at(2));
    Eigen::Quaterniond initAttitude(initPose_.at(6), initPose_.at(3), initPose_.at(4), initPose_.at(5));
    initAttitude.normalize();
    uavDynamicsSim_->setInitialPosition(initPosition, initAttitude);

    return 0;
}

int8_t Uav_Dynamics::initSensors(){
    actuatorsSub_ = node_.subscribe("/uav/actuators", 1, &Uav_Dynamics::actuatorsCallback, this);
    armSub_ = node_.subscribe("/uav/arm", 1, &Uav_Dynamics::armCallback, this);
    scenarioSub_ = node_.subscribe("/uav/scenario", 1, &Uav_Dynamics::scenarioCallback, this);
    return _sensors.init(uavDynamicsSim_);
}

int8_t Uav_Dynamics::initCalibration(){
    calibrationSub_ = node_.subscribe("/uav/calibration", 1, &Uav_Dynamics::calibrationCallback, this);
    return 0;
}

int8_t Uav_Dynamics::startClockAndThreads(){
    ros::Duration(0.1).sleep();
    if(useSimTime_){
        clockPub_ = node_.advertise<rosgraph_msgs::Clock>("/clock", 1);
        rosgraph_msgs::Clock clock_time;
        clock_time.clock = currentTime_;
        clockPub_.publish(clock_time);
    }else{
        // Get the current time if we are using wall time. Otherwise, use 0 as initial clock.
        currentTime_ = ros::Time::now();
    }


    simulationLoopTimer_ = node_.createWallTimer(ros::WallDuration(dt_secs_/clockScale_),
                                                 &Uav_Dynamics::simulationLoopTimerCallback,
                                                 this);
    simulationLoopTimer_.start();

    proceedDynamicsTask = std::thread(&Uav_Dynamics::proceedDynamics, this, dt_secs_);
    proceedDynamicsTask.detach();

    publishToRosTask = std::thread(&Uav_Dynamics::publishToRos, this, ROS_PUB_PERIOD_SEC);
    publishToRosTask.detach();

    diagnosticTask = std::thread(&Uav_Dynamics::performLogging, this, 1.0);
    diagnosticTask.detach();

    return 0;
}

/**
 * @brief Main Simulator loop
 * @param event Wall clock timer event
 */
void Uav_Dynamics::simulationLoopTimerCallback(const ros::WallTimerEvent& event){
    if (useSimTime_){
        currentTime_ += ros::Duration(dt_secs_);
        rosgraph_msgs::Clock clock_time;
        clock_time.clock = currentTime_;
        clockPub_.publish(clock_time);
    } else {
        ros::Time loopStartTime = ros::Time::now();
        dt_secs_ = (loopStartTime - currentTime_).toSec();
        currentTime_ = loopStartTime;
    }
}

std::string COLOR_RED = "\033[1;31m";
std::string COLOR_GREEN = "\033[1;32m";
std::string COLOR_BOLD = "\033[1;29m";
std::string COLOR_TAIL = "\033[0m";

void logColorizeAndAddToStream(std::stringstream& logStream, bool is_ok, std::string& newData) {
    if(!is_ok){
        logStream << COLOR_RED << newData << COLOR_TAIL;
    }else{
        logStream << newData;
    }
}

void logAddBoldStringToStream(std::stringstream& logStream, const char* newData) {
    logStream << COLOR_BOLD << newData << COLOR_TAIL;
}

void Uav_Dynamics::performLogging(double periodSec){
    while(ros::ok()){
        auto crnt_time = std::chrono::system_clock::now();
        auto sleed_period = std::chrono::seconds(int(periodSec * clockScale_));

        std::stringstream logStream;

        std::string arm_str = armed_ ? COLOR_GREEN + "[Armed]" + COLOR_TAIL : "[Disarmed]";
        logStream << arm_str << ", ";

        logStream << dynamicsTypeName_.c_str() << ". ";

        float dynamicsCompleteness = dynamicsCounter_ * dt_secs_ / (clockScale_ * periodSec);
        std::string dyn_str = "dyn=" + std::to_string(dynamicsCompleteness);
        logColorizeAndAddToStream(logStream, dynamicsCompleteness >= 0.9, dyn_str);
        logStream << ", ";
        dynamicsCounter_ = 0;

        float rosPubCompleteness = rosPubCounter_ * ROS_PUB_PERIOD_SEC / (clockScale_ * periodSec);
        std::string ros_pub_str = "ros_pub=" + std::to_string(rosPubCompleteness);
        logColorizeAndAddToStream(logStream, rosPubCompleteness >= 0.9, ros_pub_str);
        logStream << ", ";
        rosPubCounter_ = 0;

        std::string actuator_str = "setpoint=" + std::to_string(actuatorsMsgCounter_);
        bool is_actuator_ok = actuatorsMsgCounter_ > 100 && maxDelayUsec_ < 20000 && maxDelayUsec_ != 0;
        logColorizeAndAddToStream(logStream, is_actuator_ok, actuator_str);
        logStream << " msg/sec.\n";
        actuatorsMsgCounter_ = 0;
        maxDelayUsec_ = 0;

        logAddBoldStringToStream(logStream, "mc");
        logStream << std::setprecision(2) << std::fixed << " ["
                  << actuators_[0] << ", "
                  << actuators_[1] << ", "
                  << actuators_[2] << ", "
                  << actuators_[3] << "] ";

        if(vehicleType_ == VEHICLE_INNOPOLIS_VTOL){
            logAddBoldStringToStream(logStream, "fw rpy");
            logStream << " [" << actuators_[4] << ", "
                              << actuators_[5] << ", "
                              << actuators_[6] << "]";
            logAddBoldStringToStream(logStream, " throttle");
            logStream << " [" << actuators_[7] << "] ";
        }

        auto pose = uavDynamicsSim_->getVehiclePosition();
        auto enuPosition = (_dynamicsNotation == PX4_NED_FRD) ? Converter::nedToEnu(pose) : pose;
        logAddBoldStringToStream(logStream, "enu pose");
        logStream << std::setprecision(1) << std::fixed << " ["
                  << enuPosition[0] << ", "
                  << enuPosition[1] << ", "
                  << enuPosition[2] << "].";

        ROS_INFO_STREAM(logStream.str());
        fflush(stdout);
        std::this_thread::sleep_until(crnt_time + sleed_period);
    }
}

// The sequence of steps for lockstep are:
// The simulation sends a sensor message HIL_SENSOR including a timestamp time_usec to update
// the sensor state and time of PX4.
// PX4 receives this and does one iteration of state estimation, controls, etc. and eventually
// sends an actuator message HIL_ACTUATOR_CONTROLS.
// The simulation waits until it receives the actuator/motor message, then simulates the physics
// and calculates the next sensor message to send to PX4 again.
// The system starts with a "freewheeling" period where the simulation sends sensor messages
// including time and therefore runs PX4 until it has initialized and responds with an actautor
// message.
// But instead of waiting actuators cmd, we will wait for an arming
void Uav_Dynamics::proceedDynamics(double periodSec){
    while(ros::ok()){
        auto crnt_time = std::chrono::system_clock::now();
        auto sleed_period = std::chrono::milliseconds(int(1000 * periodSec * clockScale_));
        auto time_point = crnt_time + sleed_period;
        dynamicsCounter_++;

        if(calibrationType_ != UavDynamicsSimBase::CalibrationType_t::WORK_MODE){
            uavDynamicsSim_->calibrate(calibrationType_);
        }else if(armed_){
            static auto crnt_time = std::chrono::system_clock::now();
            auto prev_time = crnt_time;
            crnt_time = std::chrono::system_clock::now();
            auto time_dif_sec = (crnt_time - prev_time).count() / 1000000000.0;

            ///< prevent big time jumping
            const double MAX_TIME_DIFF_SEC = 10 * periodSec;
            if (time_dif_sec > MAX_TIME_DIFF_SEC) {
                ROS_ERROR_STREAM_THROTTLE(1, "Time jumping: " << time_dif_sec << " seconds.");
                time_dif_sec = MAX_TIME_DIFF_SEC;
            }

            uavDynamicsSim_->process(time_dif_sec, actuators_, true);
        }else{
            uavDynamicsSim_->land();
        }

        _sensors.publishStateToCommunicator(_dynamicsNotation);

        std::this_thread::sleep_until(time_point);
    }
}

void Uav_Dynamics::publishToRos(double period){
    while(ros::ok()){
        auto crnt_time = std::chrono::system_clock::now();
        auto sleed_period = std::chrono::microseconds(int(1000000 * period * clockScale_));
        auto time_point = crnt_time + sleed_period;
        rosPubCounter_++;

        _rviz_visualizator.publishTf(_dynamicsNotation);

        static auto next_time = std::chrono::system_clock::now();
        if(crnt_time > next_time){
            if (dynamicsType_ == DYNAMICS_INNO_VTOL) {
                _rviz_visualizator.publish(_dynamicsNotation);
            }
            next_time += std::chrono::milliseconds(int(50));
        }

        std::this_thread::sleep_until(time_point);
    }
}

void Uav_Dynamics::actuatorsCallback(sensor_msgs::Joy::Ptr msg){
    prevActuatorsTimestampUsec_ = lastActuatorsTimestampUsec_;
    lastActuatorsTimestampUsec_ = msg->header.stamp.toNSec() / 1000;
    auto crntDelayUsec = lastActuatorsTimestampUsec_ - prevActuatorsTimestampUsec_;
    if(crntDelayUsec > maxDelayUsec_){
        maxDelayUsec_ = crntDelayUsec;
    }
    actuatorsMsgCounter_++;

    for(size_t idx = 0; idx < msg->axes.size(); idx++){
        actuators_[idx] = msg->axes[idx];
    }

    if (_scenarioType == 1) {
        actuators_[7] = 0.0;
    }
}

void Uav_Dynamics::armCallback(std_msgs::Bool msg){
    if(armed_ != msg.data){
        /**
         * @note why it publish few times when sim starts? hack: use throttle
         */
        ROS_INFO_STREAM_THROTTLE(1, "cmd: " << (msg.data ? "Arm" : "Disarm"));
    }
    armed_ = msg.data;
}

void Uav_Dynamics::scenarioCallback(std_msgs::UInt8 msg){
    _scenarioType = msg.data;
    if (_scenarioType == 0) {
        _sensors.iceStatusSensor.stop_stall_emulation();
    } else if (_scenarioType == 1) {
        _sensors.iceStatusSensor.start_stall_emulation();
    }
}

void Uav_Dynamics::calibrationCallback(std_msgs::UInt8 msg){
    if(calibrationType_ != msg.data){
        ROS_INFO_STREAM_THROTTLE(1, "calibration type: " << msg.data + 0);
    }
    calibrationType_ = static_cast<UavDynamicsSimBase::CalibrationType_t>(msg.data);
}