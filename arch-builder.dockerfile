FROM archlinux
RUN pacman -Sy --noconfirm base-devel
ARG DOCKER_USER=arch

# Set user and group
ARG user=appuser
ARG group=appuser
ARG uid=1000
ARG gid=1000
RUN groupadd -g ${gid} ${group}
RUN useradd -u ${uid} -g ${group} -s /bin/sh -m ${user} # <--- the '-m' create a user home directory
ENV USER=${user}

# Switch to user
USER ${uid}:${gid}
