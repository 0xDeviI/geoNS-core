# Source Tree Overview

This document provides an overview of the directory structure and the purpose of each component in the [geoNS-core](https://github.com/0xDeviI/geoNS-core) repository.

## Repository Structure

### Root Directory
- **`README.md`**: The main documentation file for the repository, providing an introduction, setup instructions, and usage guidelines.
- **`Makefile`**: The build configuration file for compiling the project.
- **`debug/`**: Contains debugging-related files. Check out [Debug & Release](./debugging-and-release.md) and [Tests](./tests.md) documentations for more.

### `docs/`
Contains all documentation files for the project.
- **`README.md`**: Overview of the documentation directory.
- **`contribution.md`**: Guidelines for contributing to the project.
- **`debugging-and-release.md`**: Documentation on debugging and release processes.
- **`netforking.md`**: Explanation of the netforking mechanism.
- **`philosophy.md`**: Describes the design philosophy and principles behind the project.
- **`source-tree.md`**: This file, providing an overview of the source tree.
- **`tests.md`**: Documentation on running and writing tests.

### `src/`
Contains the source code for the project.
- **`main.c`**: The entry point of the core.
- **`libs/`**: A collection of libraries used in the project.
  - **`api/`**: API-related functionality.
  - **`argparse/`**: Command-line argument parsing.
  - **`config/`**: Configuration management.
  - **`core/`**: Core functionality of the application.
    - **`constants.h`**: Defines constants used throughout the project.
    - **`globals.h`**: Defines global variables.
    - **`types.h`**: Defines custom data types.
  - **`db/`**: Database-related functionality.
  - **`geonsp/`**: GeoNS Protocol functionality.
  - **`http/`**: HTTP-related functionality.
    - **`README.md`**: Documentation for the HTTP module.
    - **`specs.h`**: HTTP specifications and constants.
    - **`parser/`**: HTTP parsing functionality.
  - **`io/`**: Input/Output operations.
  - **`logger/`**: Logging functionality.
  - **`parson/`**: JSON parsing library.
  - **`server/`**: GeoNS Server functionality.
  - **`socket/`**: Socket-related functionality.
  - **`unity/`**: Unity testing framework.

### `tests/`
Contains test cases for the project.