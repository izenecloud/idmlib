#!/bin/bash
dir=$1
cat $dir/*.peop > $dir/r.peop
sort -u $dir/r.peop > $dir/peop
rm -rf $dir/*.peop
cat $dir/*.loc > $dir/r.loc
sort -u $dir/r.loc > $dir/loc
rm -rf $dir/*.loc
cat $dir/*.org > $dir/r.org
sort -u $dir/r.org > $dir/org
rm -rf $dir/*.org
