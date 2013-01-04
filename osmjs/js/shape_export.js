/*

  Osmium Javascript Example

  shape_export.js

  run with: osmjs -2 -m -l sparsetable -j shape_export.js OSMFILE

*/

var shp_pois = Osmium.Output.Shapefile.open('./pois', 'point');
shp_pois.add_field('id', 'string', 12);
shp_pois.add_field('type', 'string', 32);
shp_pois.add_field('name', 'string', 32);

var shp_roads = Osmium.Output.Shapefile.open('./roads', 'line');
shp_roads.add_field('id', 'integer', 10);
shp_roads.add_field('type', 'string', 32);
shp_roads.add_field('name', 'string', 32);
shp_roads.add_field('oneway', 'integer', 1);
shp_roads.add_field('maxspeed', 'integer', 3);

var shp_landuse = Osmium.Output.Shapefile.open('./landuse', 'polygon');
shp_landuse.add_field('id', 'integer', 10);
shp_landuse.add_field('type', 'string', 32);

var node_tags = {
    amenity: { restaurant: 'restaurant', pub: 'pub' },
    shop: { supermarket: 'supermarket' }
}

Osmium.Callbacks.init = function() {
    print("Init");
}

Osmium.Callbacks.node = function() {
    for (var key in this.tags) {
        if (node_tags[key]) {
            var type = node_tags[key][this.tags[key]];
            if (type) {
                shp_pois.add(this.geom, { id: this.id, type: type, name: this.tags.name });
            }
        }
    }
}

Osmium.Callbacks.way = function() {
    if (this.tags.highway) {
        var oneway = 0;
        var maxspeed = 0;
        if (this.tags.oneway == '1' || this.tags.oneway == 'yes' || this.tags.oneway == 'true') {
            oneway = 1;
        } else if (this.tags.oneway == '-1') {
            oneway = 2;
        }
        if (this.tags.maxspeed && this.tags.maxspeed.match(/^([0-9]+) ?(km\/?h)?$/)) {
            maxspeed = RegExp.$1;
        }
        shp_roads.add(this.geom, { id: this.id, type: this.tags.highway, name: this.tags.name, oneway: oneway, maxspeed: maxspeed });
    }
}

Osmium.Callbacks.area = function() {
    if (this.tags.landuse) {
        shp_landuse.add(this.geom, { id: this.id, type: this.tags.landuse });
    }
}

Osmium.Callbacks.end = function() {
    shp_pois.close();
    shp_roads.close();
    shp_landuse.close();
    print("Done");
}

