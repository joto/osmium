/* shape.js */

var shp = Osmium.Output.Shapefile.open('./pois', 'point');
shp.add_field('id', 'integer', 10);
shp.add_field('name', 'string', 32);

var lines = Osmium.Output.Shapefile.open('./lines', 'line');
lines.add_field('id', 'integer', 10);
lines.add_field('type', 'string', 32);
lines.add_field('name', 'string', 32);

var b = Osmium.Output.Shapefile.open('./buildings', 'polygon');
b.add_field('id', 'integer', 10);
b.add_field('name', 'string', 32);

function cb_node() {
//    print('node ' + this.id + ' ' + this.version + ' ' + this.timestamp + ' ' + this.uid + ' ' + this.user + ' ' + this.changeset + ' ' + this.lon + ' ' + this.lat + ' ' + this.geom.as_wkt + ' [' + this.geom.as_hex_wkb + ']');
    for (key in this.tags) {
//        print(' ' + key + '=' + this.tags[key]);
        if (key == 'amenity' && this.tags[key] == 'restaurant') {
//           print(' ' + key + '=' + this.tags.name);
           shp.add(this, { id: this.id, name: this.tags.name });
        }
    }
}

function cb_way() {
    for (key in this.tags) {
        if (key == 'highway') {
           print(' ' + key + '=' + this.tags.name);
           lines.add(this, { id: this.id, name: this.tags.name || '', type: this.tags[key] });
        }
        if (key == 'building') {
           print(' ' + key + '=' + this.tags.name);
           b.add(this, { id: this.id, name: this.tags.name || '' });
        }
    }
}

function cb_init() {
    print('Start!');
}

function cb_end() {
    b.close();
    lines.close();
    shp.close();
    print('End!');
}

Osmium.Callbacks.init     = cb_init;
Osmium.Callbacks.node     = cb_node;
Osmium.Callbacks.way      = cb_way;
Osmium.Callbacks.end      = cb_end;

