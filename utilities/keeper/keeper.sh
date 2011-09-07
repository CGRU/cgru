#!/bin/bash

pushd `dirname $0` > /dev/null
export CGRU_KEEPER=$PWD
cd ../..
[ -f "./setup.sh" ] && source ./setup.sh
popd > /dev/null

python="python"
pyver="3.2.2"
pythondir=$CGRU_KEEPER/Python
[ -d $pythondir ] || pythondir=$CGRU_LOCATION/utilities/python/$pyver
if [ -d $pythondir ]; then
   source $AF_ROOT/py3k_setup.sh $pyver
   echo "Using CGRU Python: $pythondir"
   export PATH="$pythondir/bin:$PATH"
   python="python3"
fi

"$python" "$CGRU_KEEPER/keeper.py"
