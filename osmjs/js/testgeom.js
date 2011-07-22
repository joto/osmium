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
    print("polygon_geom.toWKT(): " + this.polygon_geom.toWKT());
    print("");
}

Osmium.Callbacks.area = function() {
    print("area id:              " + this.id);
    print("geom.toWKT():         " + this.geom.toWKT());
    print("geom.toWKT(true):     " + this.geom.toWKT(true));
    print("geom.toHexWKB():      " + this.geom.toHexWKB());
    print("geom.toHexWKB(true):  " + this.geom.toHexWKB(true));
    print("geom.toArray():       " + JSON.stringify(this.geom.toArray()));
    print("");
}

