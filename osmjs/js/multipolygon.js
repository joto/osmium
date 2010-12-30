/* multipolygon.js (use with 2-pass version) */

var mp = Osmium.Output.Shapefile.open('./multipolygons', 'polygon');
mp.add_field('id', 'integer', 10);
mp.add_field('type', 'string', 32);
mp.add_field('name', 'string', 32);

Osmium.Callbacks.init = function() {
    print('Start!');
}

Osmium.Callbacks.multipolygon = function() {
    print('multipolygon ' + this.id);

    mp.add(this, {
        id:   this.id,
        name: this.tags.name,
        type: this.tags.boundary ? 'boundary' :
              this.tags.building ? 'building' :
              this.tags.highway  ? 'highway'  : 'unknown'
    });
}

Osmium.Callbacks.end = function() {
    mp.close();
    print('End!');
}

