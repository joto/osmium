/* shape.js */

var mp = Osmium.Output.Shapefile.open('./multipolygons', 'polygon');
mp.add_field('id', 'integer', 10);
mp.add_field('name', 'string', 32);

function cb_multipolygon() {
    print('mp ' + this.id);

    mp.add(this, { id: this.id, name: 'unknown' });
}

function cb_init() {
    print('Start!');
}

function cb_end() {
    mp.close();
    print('End!');
}

callbacks.init         = cb_init;
callbacks.multipolygon = cb_multipolygon;
callbacks.end          = cb_end;

