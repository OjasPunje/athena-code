# athena_science

This repository contains all packages for the Science subteam for UMD Loop's 2024-25 rover, Athena.

-----

## Guidelines
* Fork this repository for development, you can open a pull request to this repo to add your changes for everyone.
* Do NOT force push.
* Follow best practices (descriptive commit messages, well documented code, etc.)

## Versions
This code is written for an Ubuntu 22.04 system with ROS2 Humble Hawksbill and Gazebo Harmonic.

## Building

Note that to build this repository, you must have a [workspace](https://docs.ros.org/en/humble/Tutorials/Beginner-Client-Libraries/Creating-A-Workspace/Creating-A-Workspace.html). You can clone this repository into the src/ folder.

### Installing Dependencies

Inside of your workspace, run:
```bash
rosdep install --from-paths src -y --ignore-src
```
This will search through the `package.xml` files and install the necessary packages. Read more about rosdep and how to properly configure your packages [here](https://docs.ros.org/en/humble/Tutorials/Intermediate/Rosdep.html).

### Building

Inside of your workspace, run:
```bash
colcon build --packages-select  <name-of-pkg>
```

You can also do ``colcon build`` to build all packages.

After building, source your install with:

```bash
source install/setup.bash
```



### How to use Control Switching

1. **Build the package:**
```bash
colcon build --packages-select athena_science_bringup
./launch.sh
source install/setup.bash
```
2. **Open a new terminal, source the workspace, and run the controller switcher:**
```bash
source install/setup.bash
ros2 run athena_science_bringup controller_switcher.py
```

3. **Open another terminal, source the workspace, and call the service to set controllers:**
```bash
source install/setup.bash
ros2 service call /set_controller athena_science_bringup/srv/SetController "{controller_names: [INCLUDE CONTROLLER(S) YOU WANT WITHIN BRACKETS]}"
```
Example: `ros2 service call /set_controller athena_science_bringup/srv/SetController "{controller_names: [joint_group_position_controller, joint_group_velocity_controller]}"`
