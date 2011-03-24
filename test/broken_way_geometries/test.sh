#!/bin/sh
#
#  broken_way_geometries/test.sh
#

rm -f tmp/way.*
mkdir -p tmp
../osmjs/osmjs --debug --include=../osmjs/js/osm2shape.js --javascript=broken_way_geometries/broken_way_geometries.js broken_way_geometries/broken_way_geometries.osm

