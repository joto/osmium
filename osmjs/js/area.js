/*

  Osmium Javascript Example

  area.js

  run with: osmjs -2 -l sparsetable -j area.js OSMFILE

*/

var areas = Osmium.Output.Shapefile.open('./areas', 'polygon');
areas.add_field('id', 'integer', 10);
areas.add_field('type', 'string', 32);
areas.add_field('name', 'string', 32);

Osmium.Callbacks.init = function() {
    print('Start!');
}

Osmium.Callbacks.area = function() {
    print('area ' + this.id + " from " + this.from);

    areas.add(this.geom, {
        id:   this.id,
        name: this.tags.name,
        type: this.tags.boundary ? 'boundary' :
              this.tags.building ? 'building' : 'unknown'
    });
}

Osmium.Callbacks.end = function() {
    areas.close();
    print('End!');
}

