# geoNS-core
geoNS (Geolocational Net Stat) is a decentralized service that monitors internet quality. This repo contains C implementation for geoNS-core. The core service is responsible for handling decentralized operations, log collection and API provision.

## Development prerequisites
To build the project, you need to install the following development packages on your system:

### Debian Based Distros (Ubuntu, Debian, etc.)
```bash
sudo apt update
sudo apt install make clang pkg-config sqlite3 libsqlite3-dev openssl libssl-dev \
    libmagic-dev libmagic1 file libcurl4-openssl-dev
```

### RHEL Based Distros (RHEL, CentOS, Fedora, Rocky Linux, etc.)
```bash
# For RHEL/CentOS/Rocky:
sudo yum install make clang pkgconfig sqlite sqlite-devel openssl openssl-devel \
    file file-devel libcurl-devel

# For Fedora:
sudo dnf install make clang pkgconfig sqlite sqlite-devel openssl openssl-devel \
    file file-devel libcurl-devel
```

### Alpine
```bash
sudo apk update
sudo apk add make clang pkgconfig sqlite-dev openssl-dev file-dev curl-dev \
    musl-dev libmagic
```

**Note for Alpine Users:** On Alpine Linux, `libmagic-dev` is typically installed via the `file-dev` package, and `openssl-dev` provides the OpenSSL development libraries. The `musl-dev` package is required for standard C library headers when using musl libc.

## Build
In order to build the project, you need all the 'development prerequisites' packages to be installed on your system. Then you can build the core using following command:
```bash
make
```
within a successful build, you should have a 'bin/' directory that contains geons-core.

## Docs
If you want to get yourself more familiar with the project, check out [official documentation](/docs/).