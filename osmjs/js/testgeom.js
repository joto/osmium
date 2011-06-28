/*

  Osmium Javascript Example

  testgeom.js

  run with: osmjs -2 -l sparsetable -j testgeom.js OSMFILE

*/

Osmium.Callbacks.node = function() {
    print("node id:              " + this.id);
    print("geom.lon:             " + this.geom.lon);
    print("geom.lat:             " + this.geom.lat);
    print("geom.toArray():       " + JSON.stringify(this.geom.toArray()));
    print("geom.toWKT():         " + this.geom.toWKT());
    print("geom.toWKT(true):     " + this.geom.toWKT(true));
    print("geom.toHexWKB():      " + this.geom.toHexWKB());
    print("geom.toHexWKB(true):  " + this.geom.toHexWKB(true));
    print("");
}

Osmium.Callbacks.way = function() {
    print("way id:               " + this.id);
    print("geom.toArray():       " + JSON.stringify(this.geom.toArray()));
    print("geom.toWKT():         " + this.geom.toWKT());
    print("geom.toWKT(true):     " + this.geom.toWKT(true));
    print("geom.toHexWKB():      " + this.geom.toHexWKB());
    print("geom.toHexWKB(true):  " + this.geom.toHexWKB(true));
    print("reverse_geom.toWKT(): " + this.reverse_geom.toWKT());
    print("reverse_geom.toArray():       " + JSON.stringify(this.reverse_geom.toArray()));
    print("");
}

Osmium.Callbacks.multipolygon = function() {
    print("multipolygon id:      " + this.id);
    print("geom.toWKT():         " + this.geom.toWKT());
    print("geom.toWKT(true):     " + this.geom.toWKT(true));
    print("geom.toHexWKB():      " + this.geom.toHexWKB());
    print("geom.toHexWKB(true):  " + this.geom.toHexWKB(true));
    print("geom.toArray():       " + JSON.stringify(this.geom.toArray()));
    print("");
}

