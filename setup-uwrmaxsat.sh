#!/usr/bin/env bash

function notify {
  echo -e "\033[1;32m$1\033[0m"
}

cd external || exit
if [ ! -d "scipoptsuite-9.0.1" ]; then
    notify "Please download SCIP Optimization Suite 9.0.1 from:"
    notify "https://scipopt.org/download.php?fname=scipoptsuite-9.0.1.tgz"
    notify "and extract it to external/scipoptsuite-9.0.1"
    exit 1
fi

notify "Cloning CaDiCaL"
git clone --depth 1 --branch rel-1.9.5 https://github.com/arminbiere/cadical || exit
notify "Cloning CominiSat"
git clone --depth 1 https://github.com/marekpiotrow/cominisatps || exit
notify "Cloning UWrMaxSat"
git clone --depth 1 --branch v1.6.0 https://github.com/marekpiotrow/UWrMaxSat uwrmaxsat || exit
cd uwrmaxsat || exit
patch -p1 <../../uwrmaxsat.patch || exit
cd ..

notify "Building SCIP"
cd scipoptsuite-9.0.1 || exit
mkdir build
cd build || exit
cmake -DSHARED=off -DNO_EXTERNAL_CODE=on -DSOPLEX=on -DTPI=tny .. || exit
cmake --build . --config Release --target libscip libsoplex-pic || exit
cd ../..

notify "Building CaDiCaL"
cd cadical || exit
patch -p1 <../uwrmaxsat/cadical.patch || exit
./configure || exit
make || exit
cd ..

notify "Building UWrMaxSat"
cd uwrmaxsat || exit
MAXPRE="" make r || exit

notify "Success!"
