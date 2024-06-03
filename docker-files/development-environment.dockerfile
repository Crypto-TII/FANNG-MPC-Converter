# Platform information is added to support deployments on Apple Silicon but this information is orthogonal if running on x86!
FROM --platform=linux/amd64 ubuntu:20.04

USER root

ENV DEBIAN_FRONTEND=noninteractive

ENV BOOST_VERSION=1_71_0

RUN apt --allow-releaseinfo-change update && \
    apt install git wget apt-transport-https ca-certificates gnupg-agent curl build-essential g++ python-dev autotools-dev libicu-dev \
    libssl-dev libgmp3-dev make cmake zip unzip autoconf liblog4cpp5-dev zlib1g-dev libmysqlcppconn-dev \
    python3-mysqldb libcpprest-dev gcovr python3-pip libpq-dev libpqxx-dev -y

RUN pip3 install --upgrade pip && \
    pip install requests mysql mysql-connector-python psycopg2-binary fastapi[full] uvicorn[standard]

RUN cd /opt && \
    git clone https://github.com/Microsoft/vcpkg.git && \
    cd vcpkg

RUN cd /opt/vcpkg && \
    ./bootstrap-vcpkg.sh -disableMetrics
RUN cd /opt/vcpkg && git pull && git checkout a69b652 && ./vcpkg update && ./vcpkg upgrade --no-dry-run && ./vcpkg install openssl
RUN cd /opt/vcpkg && ./vcpkg install seal grpc protobuf yaml-cpp cryptopp[pem-pack] catch2 --recurse

# Removing Boost library 1.67.0 which is getting installed earlier
RUN rm -rf /usr/include/boost/

RUN wget https://github.com/Crypto-TII/boost_1_71_0/raw/main/boost_${BOOST_VERSION}.tar.gz && \
    tar xzvf boost_${BOOST_VERSION}.tar.gz && \
    cd boost_${BOOST_VERSION}/ && \
    ./bootstrap.sh --prefix=/usr/ && ./b2 && ./b2 install && \
    rm ../boost_${BOOST_VERSION}.tar.gz

RUN pip install grpcio grpcio-tools numpy

USER mt2converter
