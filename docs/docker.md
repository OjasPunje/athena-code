# Docker workflow

The root `Dockerfile` builds Athena on ROS 2 Humble and `compose.yaml` exposes
three ways to use it:

- `shell`: an interactive development shell with host networking.
- `simulation`: Gazebo, headless by default, with optional RViz/X11 support.
- `hardware`: an opt-in privileged shell with the host `/dev` tree mounted for
  CAN, serial, joystick, and camera devices.

## Prepare and build

The navigation configuration is a Git submodule and the terrain mesh uses Git
LFS, so prepare both before sending the build context to Docker:

```bash
git submodule sync --recursive
git submodule update --init --recursive
git lfs install
git lfs pull
docker compose build
```

The image installs apt dependencies through `rosdep`, Python requirements, and
MAVSDK 3.14, then runs a release `colcon` build. Ubuntu 22.04/MAVSDK packages are
selected from the container architecture, so both amd64 and supported arm64
base images use the same Dockerfile.

## Develop

Start a disposable shell:

```bash
docker compose run --rm shell
```

`src` is bind-mounted from the host. Build changes inside the container with:

```bash
build-athena-workspace
source install/setup.bash
```

The `build`, `install`, and `log` directories live in named Docker volumes. This
keeps host-owned ROS artifacts out of the repository. If an image rebuild
changes dependencies or installed packages, recreate those volumes once:

```bash
docker compose down --volumes
docker compose build
```

Use a debug build when needed:

```bash
BUILD_TYPE=Debug docker compose build
```

The YOLO node's Ultralytics/PyTorch runtime is optional because it adds several
gigabytes to the image. Include it only on machines that run that node:

```bash
INSTALL_ML=1 docker compose build
```

## Simulation and GUI

Headless Gazebo is the default:

```bash
docker compose --profile simulation up simulation
```

For Gazebo/RViz windows on a Linux X11 host:

```bash
xhost +local:docker
HEADLESS=false RVIZ=true docker compose --profile simulation up simulation
```

Revoke the broad X11 grant afterward with `xhost -local:docker`. Wayland-only,
macOS, and Windows hosts need an X server or a separate remote-desktop setup.

## Hardware and CAN

The normal shell and simulation are unprivileged. Enter the hardware profile
only when host device access is required:

```bash
docker compose --profile hardware run --rm hardware
```

The container uses host networking and mounts `/dev`, so an existing host
`can0` interface is visible inside it. Configure physical CAN on the host first,
or create `vcan0` on the host for testing. The privileged profile deliberately
is not the default because it removes most device isolation.

Set a ROS domain for any service with:

```bash
ROS_DOMAIN_ID=7 docker compose run --rm shell
```

## ZED/CUDA build

The standard image builds all packages except `zed_components`, `zed_debug`,
`zed_ros2`, `zed_wrapper`, and the ZED-message-dependent `obstacle_detection`.
Those packages require the separately distributed ZED SDK and an NVIDIA/CUDA
runtime; the repository already vendors Stereolabs' desktop and Jetson image
builders under `src/third-party/zed-ros2-wrapper/docker`.

First build the appropriate Stereolabs image using the instructions in that
directory. Then use it as Athena's base and enable the ZED packages:

```bash
docker build \
  --build-arg BASE_IMAGE=<local-zed-image-tag> \
  --build-arg ATHENA_BUILD_ZED=1 \
  --target development \
  -t athena:humble-zed .
```

Run that image with the NVIDIA runtime, host networking/IPC, the required ZED
camera devices, and the ZED settings/resources volumes described by the
vendored wrapper documentation. The standard Compose services intentionally do
not claim a GPU or grant camera access.

## Useful commands

```bash
# Run one launch command without opening a shell
docker compose run --rm shell ros2 launch drive_bringup athena_drive.launch.py use_mock_hardware:=true

# List packages from the built overlay
docker compose run --rm shell ros2 pkg list

# Remove containers and build caches owned by this Compose project
docker compose down --volumes
```
