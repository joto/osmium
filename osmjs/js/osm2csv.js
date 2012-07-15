/*

  Osmium Javascript Example

  osm2csv.js

  run with: osmjs -j osm2csv.js OSMFILE

*/

var out = Osmium.Output.CSV.open('out.csv');

Osmium.Callbacks.node = function() {
    out.print(this.id, this.tags['name'], this.tags['note']);
}

Osmium.Callbacks.end = function() {
    out.close();
}

