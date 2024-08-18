FROM debian:latest
# TODO: Remove busybox from this list once you're done experimenting (you only need vi)
RUN apt-get update && apt-get install -y \
        build-essential \
        devscripts \
        busybox \
        dh-make \
        uuid-dev \
        uuid \
        util-linux
ARG DOCKER_USER=deb

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
