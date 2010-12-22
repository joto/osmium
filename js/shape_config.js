// export osm data to shapefiles

var shp_files = {};

var files = {};

function shapefile(name) {
    var shp = {
        name: name,
        gtype: 'point',
        columns: [],
        type: function(type) {
            this.gtype = type;
            return this;
        },
        column: function(name, type, size) {
            this.columns.push({ name: name, type: type, size: size });
            return this;
        }
    };
    files[name] = shp;
    return shp;
}

var POINT = 'point';
var LINE = 'line';
var POLYGON = 'polygon';

var INTEGER = 'integer';
var STRING = 'string';

/// CONFIG

shapefile('natural_pois').
    type(POINT).
    column('id', INTEGER, 10).
    column('type', STRING, 32).
    column('name', STRING, 32);

shapefile('roads').
    type(LINE).
    column('id', INTEGER, 10).
    column('type', STRING, 32).
    column('name', STRING, 32).
    column('ref', STRING, 16).
    column('oneway', INTEGER, 1).
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

shapefile('landuse').
    type(POLYGON).
    column('id', INTEGER, 10).
    column('type', STRING, 32).
    column('name', STRING, 32);

/// CONFIG

for (var file in files) {
    print("file " + file);
    shp_files[file] = Osmium.Output.Shapefile.open('./' + file, files[file].gtype);
    for (var i=0; i < files[file].columns.length; i++) {
        var d = files[file].columns[i];
        print("  attr " + d.name + " " + d.type + " " + d.size);
        shp_files[file].add_field(d.name, d.type, d.size);
    }
}

var rules = {
    node: [],
    way: [],
    area: []
};

function rule(type, key, value) {
    if (value == '*') {
        value = null;
    }
    var rule = {
        type: type,
        key: key,
        value: value,
        output: null,
        attrs: {},
        output: function(name) {
            this.output = name;
            return this;
        },
        attr: function(attr, key) {
            if (key == null) {
                key = attr;
            }
            this.attrs[attr] = key;
            return this;
        }
    };
    rules[type].push(rule);
    return rule;
}

function node(key, value) {
    return rule('node', key, value);
}

function way(key, value) {
    return rule('way', key, value);
}

function area(key, value) {
    return rule('area', key, value);
}

/// CONFIG

node('natural').
    output('natural_pois').
        attr('type', 'natural').
        attr('name');

way('waterway').
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

area('landuse').
    output('landuse').
        attr('type', 'landuse').
        attr('name');

/// CONFIG

function build_func(key, value) {
    if (value == null) {
        return function(obj) {
            return !!obj.tags[key];
        };
    } else if (typeof(value) == 'string') {
        if (value == '*') {
            return function(obj) {
                return !!obj.tags[key];
            };
        } else if (value.match(/\|/)) {
            value = value.split('|');
        } else {
            return function(obj) {
                return obj.tags[key] && obj.tags[key] == value;
            };
        }
    }
    
    if (value instanceof Array) {
        return function(obj) {
            if (! obj.tags[key]) {
                return false;
            }
            for(var i=0; i < value.length; i++) {
                if (obj.tags[key] == value[i]) {
                    return true;
                }
            }
            return false;
        };
    } else {
        print("ERROR");
    }
}

for (var type in rules) {
    for (var i=0; i < rules[type].length; i++) {
        var rule = rules[type][i];
        rule.match = build_func(rule.key, rule.value);
    }
}

Osmium.Callbacks.init = function() {
    print("Init");
}

function tags2attributes(id, tags, attrs) {
    var obj = { id: id };
    for (var a in attrs) {
        obj[a] = tags[attrs[a]];
    }
    return obj;
}

function check(type, osm_object) {
    for (var i=0; i < rules[type].length; i++) {
        var rule = rules[type][i];
        if (rule.match(osm_object)) {
            var a = tags2attributes(osm_object.id, osm_object.tags, rule.attrs);
/*            print("attrs: ");
            for (var x in a) {
                print("  " + x + ": " + a[x]);
            }*/
            shp_files[rule.output].add(osm_object, a);
        }
    }
}

Osmium.Callbacks.node = function() {
    check('node', this);
}

Osmium.Callbacks.way = function() {
    check('way', this);
}

Osmium.Callbacks.multipolygon = function() {
    check('area', this);
}

Osmium.Callbacks.end = function() {
    for (var file in shp_files) {
        shp_files[file].close();
    }
    print("Done");
}

