FROM devosoft/empirical

USER root

COPY . /opt/directed-digital-evolution

RUN \
  pip3 install -r /opt/directed-digital-evolution/experiments/requirements.txt \
    && \
  pip3 install osfclient \
    && \
  echo "installed python requirements"

# make sure unprivileged user has access to new files in opt
# adapted from https://stackoverflow.com/a/27703359
# and https://superuser.com/a/235398
RUN \
  chgrp --recursive user /opt/directed-digital-evolution \
    && \
  chmod --recursive g+rwx /opt/directed-digital-evolution \
    && \
  echo "user granted permissions to /opt/directed-digital-evolution"

USER user

# Init & update git submodules
RUN \
  cd /opt/directed-digital-evolution \
    && \
  git submodule update --init --recursive \
    && \
  echo "download git submodules"

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

# Define default working directory.
WORKDIR /opt/directed-digital-evolution
