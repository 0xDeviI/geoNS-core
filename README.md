# geoNS-core
geoNS (Geolocational Net Stat) is a decentralized service that monitors internet quality. This repo contains C implementation for geoNS-core. The core service is responsible for handling decentralized operations, log collection and API provision.

## Development prerequisites
In order to build the project you need the following packages to be installed on your system:
1. make
2. sqlite-devel (RHEL based systems) | libsqlite3-dev (Debian based systems)
3. openssl-devel (RHEL based systems) | libssl-dev (Debian based systems)

## Build
In order to build the project, you need all the 'development prerequisites' packages to be installed on your system. Then you can build the core using following command:
```bash
make
```
within a successful build, you should have a 'bin/' directory that contains geons-core.

## Docs
If you want to get yourself more familiar with the project, check out [official documentation](/docs/).