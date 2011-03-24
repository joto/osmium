#!/bin/sh
#
#  utf8_clipping/test.sh
#

rm -f tmp/utf8.*
#../osmjs/osmjs --debug --include=../osmjs/js/osm2shape.js --javascript=utf8_clipping/utf8_clipping.js utf8_clipping/utf8_clipping.osm
../osmjs/osmjs --debug --include=utf8_clipping/osm2shape_with_debug.js --javascript=utf8_clipping/utf8_clipping.js utf8_clipping/utf8_clipping.osm

