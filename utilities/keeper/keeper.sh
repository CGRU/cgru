#!/bin/bash

pushd `dirname $0` > /dev/null
export CGRU_KEEPER=$PWD
cd ../..
[ -f "./setup.sh" ] && source ./setup.sh
popd > /dev/null

python="python"
python3=$CGRU_KEEPER/Python3
if [ -d $python3 ]; then
   pyver="3.2.2"
   source $AF_ROOT/py3k_setup.sh $pyver
   echo "Using CGRU Python: $python3"
   export PATH="$python3/bin:$PATH"
   python="python3"
fi

"$python" "$CGRU_KEEPER/keeper.py"
