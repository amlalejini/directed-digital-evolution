# Compiling and running our experiments

Here, we provide a brief guide to compiling and running our experiments.
Please file an [issue](https://github.com/amlalejini/directed-digital-evolution/issues) if something is unclear or does not work.

We document two methods of compiling and running our experiments:

1. [Manually downloading the repository and dependencies, compiling, and running on your local machine](#manual)
2. [Running inside of a Docker container](#docker)

## Manual

These instructions assume a Ubuntu-flavored Linux operating system, and they _should_ mostly work for MacOS too (but no promises there).
Otherwise, we recommend using our Docker image or using a virtual machine running linux.

You will need a C++ compiler capable of compiling C++17 code. For example, I'm using:

```
g++ (Ubuntu 11.2.0-7ubuntu2) 11.2.0
```

Next, clone this repository from GitHub.

```
git clone https://github.com/amlalejini/directed-digital-evolution
```

After cloning, `cd` into the freshly cloned repository, and run

```
git submodule update --init --recursive
```

This should download all of the third-party dependencies (into the `third-party/` directory) necessary for compiling our experiment software. From here, you should be able to compile the experiment source code by running `make` in the root directory of the repository. Edit the `PROJECT`, `MAIN_CPP`, and `THREADING` variables at the top of the Makefile to configure which experiment you're compiling and whether you compile with threading (note some data tracking will not work with threading enabled).

The configuration files used for each experiment can be found inside the particular experiment's associated directory (`experiments/[experiment-name]/hpcc/config/`).

## Docker

You can use the Dockerfile in [our repository](https://github.com/amlalejini/directed-digital-evolution/) to build a docker image locally, or you can pull the latest docker image from this DockerHub repository: [amlalejini/directed-digital-evolution](https://hub.docker.com/r/amlalejini/directed-digital-evolution).

To pull the latest docker image from DockerHub, run

```
docker pull amlalejini/directed-digital-evolution
```

Regardless of whether you built the image locally or pulled it from DockerHub, it should contain:

- all of the requisite dependencies to run our experiment software and analysis scripts
- all of our project source code (from our GitHub repository)

To run the container interactively:

```
docker run -it --entrypoint bash amlalejini/directed-digital-evolution
```