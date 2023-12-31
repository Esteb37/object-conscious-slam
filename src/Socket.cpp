#include "Socket.hpp"
#include "Utils.hpp"

namespace Robot
{

  void Socket::setup()
  {

    poseSubscription_ = this->create_subscription<Odom>(
        "/odom", rclcpp::SensorDataQoS().reliable(),
        std::bind(&Socket::poseCallback, this, std::placeholders::_1));
    goalSubscription_ = this->create_subscription<PoseMsg>(
        "/goal", rclcpp::SensorDataQoS().reliable(),
        std::bind(&Socket::goalCallback, this, std::placeholders::_1));
    lidarSubscription_ = this->create_subscription<Lidar>(
        "/scan", rclcpp::SensorDataQoS().reliable(),
        std::bind(&Socket::lidarCallback, this, std::placeholders::_1));

    // try three times
    io_service_ = std::make_shared<boost::asio::io_service>();
    socket_ = std::make_shared<tcp::socket>(*io_service_);
    acceptor_ = std::make_shared<tcp::acceptor>(*io_service_, tcp::endpoint(tcp::v4(), 12345));

    // Start accepting connections in a separate thread
    accept_thread_ = std::thread([this]()
                                 { acceptConnections(); });

    LOG(this, "Socket initialized");
  }

  void Socket::acceptConnections()
  {
    acceptor_->accept(*socket_);
    RCLCPP_INFO(get_logger(), "Accepted connection from: %s", socket_->remote_endpoint().address().to_string().c_str());

    // Start a thread to handle this connection
    std::thread([this]()
                { handleConnection(); })
        .detach();
  }

  void Socket::handleConnection()
  {

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(this->shared_from_this());

    while (rclcpp::ok())
    {

      executor.spin_some();

      // json of jsons
      nlohmann::json json = {
          {"lidar", {
                        {"angle_min", lidar_.angle_min},
                        {"angle_max", lidar_.angle_max},
                        {"angle_increment", lidar_.angle_increment},
                        {"ranges", lidar_.ranges},
                        {"range_min", lidar_.range_min},
                        {"range_max", lidar_.range_max},
                    }},
          {"pose", {
                       {"x", pose_.pose.pose.position.x},
                       {"y", pose_.pose.pose.position.y},
                       {"theta", pose_.pose.pose.orientation.z},
                   }},
          {"goal", {
                       {"x", goal_.position.x},
                       {"y", goal_.position.y},
                       {"theta", goal_.orientation.z},
                   }}};

      auto string = json.dump();

      boost::asio::write(*socket_, boost::asio::buffer(string));

      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
  }

} // namespace Robot
