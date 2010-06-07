#!/usr/bin/env bash
if [ $# -ne 2 ]
then 
    echo "usage: update-resource.sh <username> <branchname>"
    exit 0
fi
if [ "$2" != "hawk" ]
then
    echo 'now we just support hawk branch.'
    exit 0
fi
home=`dirname "$0"`
home=`cd "$home"; pwd`

datestr=`date +%Y%m%d%H%M%S`

if [ -d "$home/resource" ]
then
    a=1
else
    mkdir "$home/resource"
fi

cp -r "$home"/resource "$home/resource-$datestr"

rsync -avP "$1"@izenesoft.cn:/data/sf1r-resource/"$2"/resource/nec/ "$home"/resource/nec/
rsync -avP "$1"@izenesoft.cn:/data/sf1r-resource/"$2"/resource/kpe/ "$home"/resource/kpe/

