/* shape.js */

var mp = Osmium.Output.Shapefile.open('./multipolygons', 'polygon');
mp.add_field('id', 'integer', 10);
mp.add_field('type', 'string', 32);
mp.add_field('name', 'string', 32);

function cb_multipolygon() {
    print('mp ' + this.id);

    mp.add(this, { id: this.id, name: this.tags.name, type: this.tags.boundary ? 'boundary' : this.tags.building ? 'building' : this.tags.highway ? 'highway' : 'unknown' });
}

function cb_init() {
    print('Start!');
}

function cb_end() {
    mp.close();
    print('End!');
}

Osmium.Callbacks.init         = cb_init;
Osmium.Callbacks.multipolygon = cb_multipolygon;
Osmium.Callbacks.end          = cb_end;

