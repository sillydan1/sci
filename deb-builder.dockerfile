FROM debian:latest
RUN apt-get update && apt-get install -y \
        build-essential \
        devscripts \
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
