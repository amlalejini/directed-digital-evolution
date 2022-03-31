# ======= USAGE =======
# Run interactively:
#   docker run -it --entrypoint bash dockertest:latest

# Pull a base image
FROM ubuntu:20.04

COPY . /opt/directed-digital-evolution

# To make installs not ask questions about timezones
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=America/New_York

##############################
# install base dependencies
# - for R repository
#   - dirmngr
#   - gpg-agent
# - for bookdown compilation
#   - pandoc, pandoc-citeproc, texlive-base, texlive-latex-extra
##############################
RUN \
  apt-get update \
    && \
  apt-get install -y -qq --no-install-recommends \
    software-properties-common \
    curl \
    g++-10 \
    make  \
    cmake \
    python3 \
    python3-pip \
    python3-virtualenv \
    git \
    libssl-dev \
    libcurl4-openssl-dev \
    libxml2-dev \
    libz-dev \
    libgit2-dev \
    libpng-dev \
    libfontconfig1-dev \
    libmagick++-dev \
    libgdal-dev \
    libharfbuzz-dev \
    libfribidi-dev \
    liblapack-dev \
    libblas-dev \
    libstdc++-10-dev \
    dirmngr \
    gpg-agent \
    pandoc \
    pandoc-citeproc \
    texlive-base \
    texlive-latex-extra \
    lmodern \
    && \
  echo "installed base dependencies"

# Wire g++ command to g++-10
RUN cd /usr/bin/ && ln -s g++-10 g++ && cd /

RUN \
  pip3 install -r /opt/directed-digital-evolution/experiments/requirements.txt \
    && \
  pip3 install osfclient \
    && \
  echo "installed python requirements"

# Init & update git submodules
RUN \
  cd /opt/directed-digital-evolution \
    && \
  git submodule update --init --recursive \
    && \
  echo "download git submodules"

# Build experiment
RUN \
  cd /opt/directed-digital-evolution \
    && \
  ./build_exps.sh \
    && \
  echo "build experiment software"

########################################################
# install r
# - source: https://rtask.thinkr.fr/installation-of-r-4-0-on-ubuntu-20-04-lts-and-tips-for-spatial-packages/
########################################################
RUN \
  gpg --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9 \
    && \
  gpg -a --export E298A3A825C0D65DFD57CBB651716619E084DAB9 | apt-key add - \
    && \
  apt update \
    && \
  add-apt-repository 'deb https://cloud.r-project.org/bin/linux/ubuntu focal-cran40/' \
    && \
  apt-get install -y -q --no-install-recommends \
    r-base \
    r-base-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    libfreetype6-dev \
    libmagick++-dev \
    libxml2-dev \
    libfontconfig1-dev \
    cargo \
    && \
  R -e "install.packages('rmarkdown', dependencies=NA, repos='http://cran.rstudio.com/')" \
    && \
  R -e "install.packages('knitr', dependencies=NA, repos='http://cran.rstudio.com/')" \
    && \
  R -e "install.packages('bookdown', dependencies=NA, repos='http://cran.rstudio.com/')" \
    && \
  R -e "install.packages('tidyverse', dependencies=NA, repos='http://cran.rstudio.com/')" \
    && \
  R -e "install.packages('cowplot', dependencies=NA, repos='http://cran.rstudio.com/')" \
    && \
  R -e "install.packages('plyr', dependencies=NA, repos='http://cran.rstudio.com/')" \
    && \
  R -e "install.packages('scales', dependencies=NA, repos='http://cran.rstudio.com/')" \
    && \
  R -e "install.packages('RColorBrewer', dependencies=NA, repos='http://cran.rstudio.com/')" \
    && \
  echo "installed r and configured r environment"

########################################################
# download data needed to compile supplement
########################################################
RUN \
  export OSF_PROJECT=zn63x \
    && \
  export PROJECT_PATH=/opt/directed-digital-evolution \
    && \
  cd ${PROJECT_PATH} \
    && \
  export EXP_TAG=2021-11-15-ec \
    && \
  ./download_data.sh \
    && \
  export EXP_TAG=2021-11-11-selection \
    && \
  ./download_data.sh \
    && \
  export EXP_TAG=2021-11-30-aligned-tasks \
    && \
  ./download_data.sh \
    && \
  export EXP_TAG=2021-11-12-time \
    && \
  ./download_data.sh \
    && \
  export EXP_TAG=2021-11-16-pops \
    && \
  ./download_data.sh \
    && \
  echo "downloaded experiment data"

########################################################
# build supplemental material (will also run data analyses)
########################################################
RUN \
  cd /opt/directed-digital-evolution/ \
    && \
  ./build_book.sh \
    && \
  echo "ran analyses and built supplemental material"

# Define default working directory.
WORKDIR /opt/directed-digital-evolution
