#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <boost/asioss.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/macros.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>

#include "Utils.hpp"
#include "json.hpp"

using boost::asio::ip::tcp;

namespace Robot
{

  class Socket : public Node
  {
  public:
    Socket() : Node("Socket")
    {
      setup();
    }

    ~Socket()
    {
      acceptor_->close();
      io_service_->stop();
      accept_thread_.join();
    }

  private:
    void setup();
    void acceptConnections();
    void handleConnection();

    void poseCallback(const Odom::SharedPtr msg)
    {
      pose_ = *msg;
    }

    void goalCallback(const PoseMsg::SharedPtr msg)
    {
      goal_ = *msg;
    }

    void lidarCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
    {
      lidar_ = *msg;
    }

    Subscription<Odom>::SharedPtr
        poseSubscription_;
    Subscription<PoseMsg>::SharedPtr
        goalSubscription_;
    Subscription<Lidar>::SharedPtr
        lidarSubscription_;

    Odom pose_;
    PoseMsg goal_;
    Lidar lidar_;

    std::shared_ptr<boost::asio::io_service> io_service_;
    std::shared_ptr<tcp::acceptor> acceptor_;
    std::shared_ptr<tcp::socket> socket_;
    std::thread accept_thread_;
  };

}
#endif