# syntax=docker/dockerfile:1.7
ARG ROS_DISTRO=humble
ARG BASE_IMAGE=osrf/ros:${ROS_DISTRO}-desktop

FROM ${BASE_IMAGE} AS dependencies

ARG ROS_DISTRO
ARG ATHENA_BUILD_ZED=0
ARG INSTALL_ML=0
ARG DEBIAN_FRONTEND=noninteractive

ENV ROS_DISTRO=${ROS_DISTRO} \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    PIP_DISABLE_PIP_VERSION_CHECK=1 \
    PYTHONUNBUFFERED=1

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
WORKDIR /workspace

RUN apt-get update && apt-get install -y --no-install-recommends \
      bash-completion \
      build-essential \
      can-utils \
      git \
      git-lfs \
      iproute2 \
      python3-colcon-common-extensions \
      python3-pip \
      python3-rosdep \
      usbutils \
      wget \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt requirements-ml.txt dependencies.sh libmavsdk-dev_*.deb /tmp/athena-dependencies/
RUN cd /tmp/athena-dependencies \
    && apt-get update \
    && ./dependencies.sh \
    && python3 -m pip install --no-cache-dir -r requirements.txt \
    && if [[ "${INSTALL_ML}" == "1" ]]; then python3 -m pip install --no-cache-dir -r requirements-ml.txt; fi \
    && rm -rf /var/lib/apt/lists/* /tmp/athena-dependencies

COPY src/ src/

# The ZED wrapper needs the separately distributed ZED SDK/CUDA image. Everything
# else is resolved here. -r lets rosdep report optional/unresolved ZED keys while
# continuing to install the dependencies for the standard workspace.
RUN rosdep update \
    && apt-get update \
    && if [[ "${ATHENA_BUILD_ZED}" == "1" ]]; then \
         rosdep install --from-paths src --ignore-src -r -y \
           --skip-keys=ament_python \
           --skip-keys=python3-pygeomag; \
       else \
         rosdep install --from-paths src --ignore-src -r -y \
           --skip-keys=ament_python \
           --skip-keys=python3-pygeomag \
           --skip-keys=zed_sdk \
           --skip-keys=zed_msgs; \
       fi \
    && rm -rf /var/lib/apt/lists/*

COPY docker/build-workspace.sh /usr/local/bin/build-athena-workspace
RUN chmod 755 /usr/local/bin/build-athena-workspace

FROM dependencies AS build

ARG BUILD_TYPE=Release
ARG ATHENA_BUILD_ZED=0

RUN ATHENA_BUILD_ZED="${ATHENA_BUILD_ZED}" BUILD_TYPE="${BUILD_TYPE}" build-athena-workspace

FROM build AS development

COPY docker/entrypoint.sh /usr/local/bin/athena-entrypoint
RUN chmod 755 /usr/local/bin/athena-entrypoint

ENTRYPOINT ["/usr/local/bin/athena-entrypoint"]
CMD ["bash"]

FROM development AS runtime
