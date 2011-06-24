/* test geom output */

Osmium.Callbacks.node = function() {
    print("node id:         " + this.id);
    print("geom.lon:        " + this.geom.lon);
    print("geom.lat:        " + this.geom.lat);
    print("geom.as_array:   " + JSON.stringify(this.geom.as_array));
    print("geom.as_wkt:     " + this.geom.as_wkt);
    print("geom.as_ewkt:    " + this.geom.as_ewkt);
    print("geom.as_hex_wkb: " + this.geom.as_hex_wkb);
    print("");
}

Osmium.Callbacks.way = function() {
    print("way id:                " + this.id);
    print("geom.as_array:         " + JSON.stringify(this.geom.as_array));
    print("geom.as_wkt:           " + this.geom.as_wkt);
    print("geom.as_ewkt:          " + this.geom.as_ewkt);
//    print("geom.as_hex_wkb:       " + this.geom.as_hex_wkb);
    print("reverse_geom.as_wkt:   " + this.reverse_geom.as_wkt);
    print("");
}

Osmium.Callbacks.multipolygon = function() {
    print("multipolygon id: " + this.id);
    print("geom.as_array:   " + JSON.stringify(this.geom.as_array));
    print("geom.as_wkt:     " + this.geom.as_wkt);
    print("geom.as_ewkt:    " + this.geom.as_ewkt);
    print("");
}

