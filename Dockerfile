# =========================================================
# Stage: build — compile worldserver + bnetserver
# =========================================================
FROM ubuntu:24.04 AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    gcc-13 \
    g++-13 \
    ninja-build \
    git \
    ca-certificates \
    libboost-all-dev \
    libssl-dev \
    libmysqlclient-dev \
    libreadline-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

WORKDIR /src
COPY . .

RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/opt/arguscore \
    -DSCRIPTS=static \
    -DTOOLS=OFF \
    -DSERVERS=ON \
    && cmake --build build --parallel "$(nproc)" \
    && cmake --install build

# =========================================================
# Stage: tdb-download — downloads and extracts TDB (world + hotfixes)
# Completely independent from the build stage, no compilation required.
# =========================================================
FROM ubuntu:24.04 AS tdb-download

RUN apt-get update && apt-get install -y --no-install-recommends curl p7zip ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tdb

RUN curl -fL -o tdb.7z \
      "https://github.com/The-Legion-Preservation-Project/TrinityCore/releases/download/TDB735.25051/TDB_full_735.26972_2025_05_11.7z" \
    && 7z x tdb.7z -o/tdb/sql

# =========================================================
# Stage: runtime-base — shared runtime layer with no default command
# =========================================================
FROM ubuntu:24.04 AS runtime-base

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libmysqlclient21 \
    mysql-client \
    libssl3 \
    libboost-system1.83.0 \
    libboost-filesystem1.83.0 \
    libboost-program-options1.83.0 \
    libboost-locale1.83.0 \
    libboost-iostreams1.83.0 \
    libreadline8 \
    zlib1g \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    && usermod -l argus -d /home/argus -m ubuntu \
    && groupmod -n argus ubuntu

COPY --from=build /opt/arguscore /opt/arguscore

RUN mkdir -p /opt/arguscore/data /opt/arguscore/etc \
    && chown -R argus:argus /opt/arguscore

ENV PATH="/opt/arguscore/bin:${PATH}"
WORKDIR /opt/arguscore
USER argus

# =========================================================
# Stage: worldserver — final target for the world service
# =========================================================
FROM runtime-base AS worldserver

USER argus

EXPOSE 8085
CMD ["worldserver", "-c", "/opt/arguscore/etc/worldserver.conf"]

# =========================================================
# Stage: bnetserver — final target for the battle.net service
# =========================================================
FROM runtime-base AS bnetserver

EXPOSE 1119 8081
CMD ["bnetserver", "-c", "/opt/arguscore/etc/bnetserver.conf"]

# =========================================================
# Stage: db-init — lightweight target for DB initialization
# Does not use the build stage at all, only copies its script.
# =========================================================
FROM ubuntu:24.04 AS db-init

RUN apt-get update && apt-get install -y --no-install-recommends bash mysql-client findutils

WORKDIR /app
COPY --from=tdb-download /tdb/sql /app/tdb
COPY db-init/init-db.sh /app/init-db.sh
RUN chmod +x /app/init-db.sh

ENTRYPOINT ["/app/init-db.sh"]