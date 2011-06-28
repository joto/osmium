/*

  Osmium Javascript Example

  multipolygon.js

  run with: osmjs -2 -l sparsetable -j multipolygon.js OSMFILE

*/

var mp = Osmium.Output.Shapefile.open('./multipolygons', 'polygon');
mp.add_field('id', 'integer', 10);
mp.add_field('type', 'string', 32);
mp.add_field('name', 'string', 32);

Osmium.Callbacks.init = function() {
    print('Start!');
}

Osmium.Callbacks.multipolygon = function() {
    print('multipolygon ' + this.id);

    mp.add(this.geom, {
        id:   this.id,
        name: this.tags.name,
        type: this.tags.boundary ? 'boundary' :
              this.tags.building ? 'building' : 'unknown'
    });
}

Osmium.Callbacks.end = function() {
    mp.close();
    print('End!');
}

