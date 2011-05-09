/* test geom output */

Osmium.Callbacks.node = function() {
    print("geom.lon:        " + this.geom.lon);
    print("geom.lat:        " + this.geom.lat);
    print("geom.x:          " + this.geom.x);
    print("geom.y:          " + this.geom.y);
    print("geom.as_array:   " + JSON.stringify(this.geom.as_array));
    print("geom.as_wkt:     " + this.geom.as_wkt);
    print("geom.as_ewkt:    " + this.geom.as_ewkt);
    print("geom.as_hex_wkb: " + this.geom.as_hex_wkb);
    print("");
}

Osmium.Callbacks.way = function() {
    print("geom.as_array:         " + JSON.stringify(this.geom.as_array));
    print("geom.as_wkt:           " + this.geom.as_wkt);
    print("geom.as_ewkt:          " + this.geom.as_ewkt);
    print("geom.as_hex_wkb:       " + this.geom.as_hex_wkb);
    print("geom.as_polygon_array: " + JSON.stringify(this.geom.as_polygon_array));
    print("");
}

Osmium.Callbacks.multipolygon = function() {
    print("geom.as_array:         " + JSON.stringify(this.geom.as_array));
    print("geom.as_wkt:           " + this.geom.as_wkt);
    print("geom.as_ewkt:          " + this.geom.as_ewkt);
    print("");
}

