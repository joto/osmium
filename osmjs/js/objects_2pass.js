/*

  Osmium Javascript Example

  objects_2pass.js

  run with: osmjs -2 -m -l sparsetable -j objects_2pass.js OSMFILE

*/

Osmium.Callbacks.init = function() {
    print('Start!');
}

Osmium.Callbacks.node = function() {
    print('node ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset + ' ' + this.geom.lon + ' ' + this.geom.lat + ' ' + this.geom.toWKT() + ' [' + this.geom.toHexWKB() + ']');
    for (var key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
    }
}

Osmium.Callbacks.way = function() {
    print('way ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset + ' ' + this.geom.toWKT());
    for (var key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
    }
    for (var i=0; i < this.nodes.length; i++) {
        print(' ref ' + this.nodes[i]);
    }
}

Osmium.Callbacks.relation = function() {
    print('relation ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset);
    for (var key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
    }
    for (var i=0; i < this.members.length; i++) {
        m = this.members[i];
        print(' member ' + m.type + ' ' + m.ref + ' ' + m.role);
    }
}

Osmium.Callbacks.area = function() {
    print('area from ' + this.from + ' ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.changeset);
    for (var key in this.tags) {
        print(' ' + key + '=' + this.tags[key]);
    }
}

Osmium.Callbacks.end = function() {
    print('End!');
}

