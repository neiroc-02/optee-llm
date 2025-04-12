#!/bin/env bash

cp -r optee_llm /opt/optee/Linux_for_Tegra/sources/tegra/optee-src/nv-optee/optee/samples

cd /opt/optee/Linux_for_Tegra/sources/tegra/optee-src/nv-optee/

./optee_src_build.sh -p t234

cd /opt/optee/Linux_for_Tegra/sources/tegra/optee-src/nv-optee/optee/install/t234/usr/sbin

cp optee_llm ~/Desktop/optee-llm

cd /opt/optee/Linux_for_Tegra/sources/tegra/optee-src/nv-optee/optee/build/t234/ta/optee_llm

cp 522* ~/Desktop/optee-llm

cd ~/Research/optee-llm
