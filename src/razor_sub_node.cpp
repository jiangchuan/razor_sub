#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <stdio.h>
#include <sys/stat.h>

#include <fstream>
#include <sstream>

std::string imu_dir;
std::string imu_timedir;
std::stringstream imustream;
int imucount = 1;
time_t lastT = time(0);
int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

time_t get_time() {
    time_t now = time(0);
    tm* ltm = gmtime(&now);
    year = 1900 + ltm->tm_year;
    month = 1 + ltm->tm_mon;
    day = ltm->tm_mday;
    hour = ltm->tm_hour;
    minute = ltm->tm_min;
    second = ltm->tm_sec;
    return now;
}

std::string digit2str(int num) {
    if (num < 10)
        return "0" + std::to_string(num);
    return std::to_string(num);
}

std::string get_time_str() {
    return std::to_string(year) + "-" + digit2str(month) + "-" + digit2str(day) + "_" + digit2str(hour);
}

void imu_callback(const sensor_msgs::Imu::ConstPtr& imu_msg) {
    if (imucount % 500 == 0) {
        time_t now = get_time();
        std::string time_str = get_time_str();
        if (difftime(now, lastT) > 3600) {
            imu_timedir = imu_dir + time_str + "/";
            int status = mkdir(imu_timedir.c_str(), 0777);
            lastT = now;
        }

        std::string imu_filename = imu_timedir + time_str + "-" + digit2str(minute) + "-" + digit2str(second) + ".csv";
        std::fstream file;
        file.open(imu_filename, std::ios::out | std::ios::app);
        file << imustream.rdbuf();
        file.close();
        imustream.str(std::string());  // clear stream
    }

    double imu_t = imu_msg->header.stamp.sec * 1.0 + imu_msg->header.stamp.nsec / 1000000000.0;
    imustream << std::setprecision(18) << imu_t << ",";
    imustream << std::setprecision(4) << imu_msg->orientation.x << ",";
    imustream << std::setprecision(4) << imu_msg->orientation.y << ",";
    imustream << std::setprecision(4) << imu_msg->orientation.z << ",";
    imustream << std::setprecision(4) << imu_msg->orientation.w << "\n";
    imucount++;
}

int main(int argc, char** argv) {
    std::string root_dir = "/home/ubuntu/livox_data/";
    // std::string root_dir = "/home/jiangchuan/livox_data/";
    imu_dir = root_dir + "imu/";

    time_t now = get_time();
    std::string time_str = get_time_str();
    imu_timedir = imu_dir + time_str + "/";

    int status = mkdir(root_dir.c_str(), 0777);
    status = mkdir(imu_dir.c_str(), 0777);
    status = mkdir(imu_timedir.c_str(), 0777);

    ros::init(argc, argv, "razor_sub");
    ros::NodeHandle nh;
    ros::Subscriber imu_sub = nh.subscribe<sensor_msgs::Imu>("imu", 100, imu_callback);  // Razor IMU
    // ros::Subscriber imu_sub = nh.subscribe<sensor_msgs::Imu>("imu/data", 100, imu_callback);     // Adis IMU
    ros::spin();

    return 0;
}