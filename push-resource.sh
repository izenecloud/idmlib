#!/usr/bin/env bash
if [ $# -ne 2 ]
then 
    echo "usage: push-resource.sh <username> <branchname>"
    exit 0
fi
if [ "$2" != "hawk" ]
then
    echo 'now we just support hawk branch.'
    exit 0
fi
home=`dirname "$0"`
home=`cd "$home"; pwd`
rsync -avP "$home"/resource/nec/ "$1"@izenesoft.cn:/data/sf1r-resource/"$2"/resource/nec/
rsync -avP "$home"/resource/kpe/ "$1"@izenesoft.cn:/data/sf1r-resource/"$2"/resource/kpe/

