=======
#SCOT
###The Stanford Circuit Optimization Tool

    git clone https://github.com/etep/scot
    cd scot
    export SCOT_HOME_DIR=`pwd`
    mkdir tmp
    cd tmp
    export SCOT_TEMP_DIR=`pwd`
    cd ../pys
    ./build.py
    ./scot.py ../hsp/invChain3.hsp
