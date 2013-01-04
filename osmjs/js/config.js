/*

  Osmium Javascript Example

  config.js

  Uses osm2shape framework for exporting OSM data to shapefiles.
  (see also osm2shape.js)

  run with: osmjs -2 -m -l sparsetable -i osm2shape.js -j config.js OSMFILE

*/

// ---- shapefiles ----

shapefile('natural_pois').
    type(POINT).
    column('id', STRING, 12).
    column('type', STRING, 32).
    column('name', STRING, 32);

shapefile('roads').
    type(LINE).
    column('id', INTEGER, 10).
    column('type', STRING, 32).
    column('name', STRING, 32).
    column('ref', STRING, 16).
    column('oneway', BOOL).
    column('maxspeed', INTEGER, 3);

shapefile('cycleways').
    type(LINE).
    column('id', INTEGER, 10).
    column('name', STRING, 32);

shapefile('railways').
    type(LINE).
    column('id', INTEGER, 10).
    column('name', STRING, 32);

shapefile('waterways').
    type(LINE).
    column('id', INTEGER, 10).
    column('type', STRING, 32).
    column('name', STRING, 32);

shapefile('powerlines').
    type(LINE).
    column('id', INTEGER, 10).
    column('name', STRING, 32);

shapefile('boundaries').
    type(POLYGON).
    column('id', INTEGER, 10).
    column('level', INTEGER, 2).
    column('name', STRING, 32);

shapefile('protected_areas').
    type(POLYGON).
    column('id', INTEGER, 10).
    column('type', STRING, 32).
    column('name', STRING, 32);

shapefile('landuse').
    type(POLYGON).
    column('id', INTEGER, 10).
    column('type', STRING, 32).
    column('name', STRING, 32);

shapefile('water').
    type(POLYGON).
    column('id', INTEGER, 10).
    column('type', STRING, 32).
    column('name', STRING, 32);

shapefile('glaciers').
    type(POLYGON).
    column('id', INTEGER, 10).
    column('name', STRING, 32);

// ---- rules ----

node('natural', 'tree|peak|spring').
    output('natural_pois').
        attr('type', 'natural').
        attr('name');

way('waterway', 'stream|river|ditch|canal|drain').
    output('waterways').
        attr('type', 'waterway').
        attr('name');

way('highway', 'motorway|trunk|primary|secondary').
    output('roads').
        attr('type', 'highway').
        attr('ref').
        attr('name').
        attr('oneway').
        attr('maxspeed');

way('highway', 'cycleway').
    output('cycleways').
        attr('name');

way('railway', 'rail').
    output('railways').
        attr('name');

way('power', 'line').
    output('powerlines').
        attr('name');

area('boundary', 'administrative').
    output('boundaries').
        attr('level', 'admin_level').
        attr('name');

area('boundary', 'national_park|national_forest|protected_area').
    output('protected_areas').
        attr('type', 'boundary').
        attr('name');

area('landuse', 'forest|grass|residential|farm|meadow|farmland|industrial|farmyard|cemetery|commercial|quarry|orchard|vineyard|allotments|retail|construction|recreation_ground|village_green').
    output('landuse').
        attr('type', 'landuse').
        attr('name');

area('natural', 'water').
    output('water').
        attr('type', 'natural').
        attr('name');

area('landuse', 'reservoir').
    output('water').
        attr('type', 'landuse').
        attr('name');

area('waterway', 'riverbank').
    output('water').
        attr('type', 'waterway').
        attr('name');

area('natural', 'glacier').
    output('glaciers').
        attr('name');

