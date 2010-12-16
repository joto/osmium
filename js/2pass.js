
/* test.js */

var out = Osmium.Output.CSV.open("pois.tbf");

function cb_node() {
    print('node ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset + ' ' + this.lon + ' ' + this.lat + ' ' + this.geom.as_wkt + ' [' + this.geom.as_hex_wkb + ']');
    for (key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
/*        if (key == 'amenity' && this.tags[key] == 'restaurant') {
           print(' ' + key + '=' + this.tags.name);
            out.print(this.id, this.lon, this.lat, this.tags.name);
        }*/
    }
}

function cb_way() {
    print('way ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset + ' ' + this.geom.linestring_wkt);
    for (key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
    }
    for (i in this.nodes) {
        print(' ref ' + this.nodes[i]);
    }
}

function cb_relation() {
    print('relation ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset);
    for (key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
    }
    for (i in this.members) {
        m = this.members[i];
        print(' member ' + m.type + ' ' + m.ref + ' ' + m.role);
    }
}

function cb_multipolygon() {
    print('multipolygon ' + this.id);
}

function cb_init() {
    print('Start!');
}

function cb_end() {
    print('End!');
    out.close();
}

callbacks.init         = cb_init;
callbacks.node         = cb_node;
callbacks.way          = cb_way;
callbacks.relation     = cb_relation;
callbacks.multipolygon = cb_multipolygon;
callbacks.end          = cb_end;

