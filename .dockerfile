# TODO: use alpine when available
FROM debian:12-slim
ADD artifacts.tar.gz /install
RUN dpkg -i /install/artifacts/sci_*-1_amd64.deb
RUN rm -rf /install
ENTRYPOINT ["sci"]
