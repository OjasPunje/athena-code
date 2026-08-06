#!/usr/bin/env bash
set -eo pipefail

source "/opt/ros/${ROS_DISTRO:-humble}/setup.bash"
set -u

build_args=(
    --symlink-install
    --cmake-args "-DCMAKE_BUILD_TYPE=${BUILD_TYPE:-Release}"
)

if [[ "${ATHENA_BUILD_ZED:-0}" != "1" ]]; then
    build_args+=(
        --packages-skip
        zed_components
        zed_debug
        zed_ros2
        zed_wrapper
        obstacle_detection
    )
fi

colcon build "${build_args[@]}" "$@"
