/* objects.js (single pass) */

Osmium.Callbacks.init = function() {
    print('Start!');
}

Osmium.Callbacks.node = function() {
    print('node ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset + ' ' + this.geom.lon + ' ' + this.geom.lat + ' ' + this.geom.as_wkt + ' [' + this.geom.as_hex_wkb + ']');
    for (key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
    }
}

Osmium.Callbacks.way = function() {
    print('way ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset + ' ' + this.geom.as_wkt);
    for (key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
    }
    for (var i=0; i < this.nodes.length; i++) {
        print(' ref ' + this.nodes[i]);
    }
}

Osmium.Callbacks.relation = function() {
    print('relation ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset);
    for (key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
    }
    for (var i=0; i < this.members.length; i++) {
        m = this.members[i];
        print(' member ' + m.type + ' ' + m.ref + ' ' + m.role);
    }
}

Osmium.Callbacks.end = function() {
    print('End!');
}

