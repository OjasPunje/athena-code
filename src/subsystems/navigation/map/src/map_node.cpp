#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <memory>

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::Node::SharedPtr node = std::make_shared<rclcpp::Node>("static_occupancy_map_demo");
  auto publisher = node->create_publisher<nav_msgs::msg::OccupancyGrid>(
    "occupancy_map", rclcpp::QoS(1).transient_local());

  auto message = std::make_unique<nav_msgs::msg::OccupancyGrid>();
  message->header.frame_id = "map";
  
  double resolution = 0.03;  // 3cm resolution
  double width = 2.0;       // 2.0m width
  double height = 1.2;      // 1.2m height
  
  message->info.resolution = resolution;
  message->info.width = static_cast<uint32_t>(width / resolution);
  message->info.height = static_cast<uint32_t>(height / resolution);
  
  message->info.origin.position.x = -width/2;
  message->info.origin.position.y = -height/2;
  message->info.origin.position.z = 0.0;
  message->info.origin.orientation.w = 1.0;

  message->data.resize(message->info.width * message->info.height);

  const int border_width = 5;

  for (uint32_t i = 0; i < message->info.height; ++i) {
    for (uint32_t j = 0; j < message->info.width; ++j) {
      // Check if cell is in the border region
      bool is_border = (i < border_width) ||                          // Top border
                      (i >= message->info.height - border_width) ||   // Bottom border
                      (j < border_width) ||                          // Left border
                      (j >= message->info.width - border_width);     // Right border
      
      message->data[i * message->info.width + j] = is_border ? 100 : 0;
    }
  }

  RCLCPP_INFO(
    node->get_logger(),
    "Created occupancy map with size %f x %f m (%u x %u cells), border width: %d cells",
    width, height, message->info.width, message->info.height, border_width);

  rclcpp::Rate rate(1.0);
  while (rclcpp::ok()) {
    message->header.stamp = node->now();
    publisher->publish(*message);
    rate.sleep();
  }

  return 0;
}