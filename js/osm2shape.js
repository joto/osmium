// framework for exporting OSM data to shapefiles

// shapefile geometry types
var POINT   = 'point';
var LINE    = 'line';
var POLYGON = 'polygon';

// shapefile attribute types
var INTEGER = 'integer';
var STRING  = 'string';
var DOUBLE  = 'double';
var BOOL    = 'bool';

var files = {};

var rules = {
    node: [],
    way:  [],
    area: []
};

function shapefile(name) {
    var shp = {
        name: name,
        fname: name,
        gtype: 'point',
        columns: [],
        column_names: {},
        type: function(type) {
            if (type != 'point' && type != 'line' && type != 'polygon') {
                print('Unknown shapefile geometry type: ' + type);
                exit(1);
            }
            this.gtype = type;
            return this;
        },
        column: function(name, type, size) {
            if (type != 'integer' && type != 'string' && type != 'bool' && type != 'double') {
                print('Unknown attribute type: ' + type);
                throw("config error");
            }
            if (size == null) {
                size = 1;
            }
            if (size < 0) {
                print('Size not allowed: ' + size);
            }
            var column = { name: name, type: type, size: size };
            this.columns.push(column);
            this.column_names[name] = column;
            return this;
        },
        filename: function(name) {
            this.fname = name;
            return this;
        }
    };
    files[name] = shp;
    return shp;
}


function rule(type, key, value) {
    if (value == '*') {
        value = null;
    }
    var rule = {
        type: type,
        key: key,
        value: value,
        file: null,
        attrs: {},
        output: function(name) {
            if (! files[name]) {
                print("Unknown shapefile: " + name);
                throw("config error");
            }
            this.file = name;
            return this;
        },
        attr: function(attr, key) {
            if (this.file == null) {
                print("Output file not set for rule " + key + '=' + value);
                throw("config error");
            }

            if (! files[this.file].column_names[attr]) {
                print("There is no column named '" + attr + "' in output file '" + this.file + "'");
                throw("config error");
            }

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

Osmium.Callbacks.init = function() {
    print("Init");

    for (var file in files) {
        var f = files[file];

        f.shp = Osmium.Output.Shapefile.open('./' + f.fname, f.gtype);

        print('Shapefile: ' + file);
        print('  Filename: ' + f.fname);
        print('  Geometry type: ' + f.gtype.toUpperCase());
        print('  Columns:');

        for (var i=0; i < f.columns.length; i++) {
            var d = f.columns[i];
            print('    ' + (d.name + '          ').substr(0, 11) + d.type.toUpperCase() + ' ' + d.size);
            f.shp.add_field(d.name, d.type, d.size);
        }

        print('');
    }

    for (var type in rules) {
        for (var i=0; i < rules[type].length; i++) {
            var rule = rules[type][i];
            if (rule.file && files[rule.file]) {
                rule.match = build_func(rule.key, rule.value);
            } else {
                print("Unknown shapefile output: " + rule.file);
                exit(1);
            }
        }
    }
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
            files[rule.file].shp.add(osm_object, a);
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
    for (var file in files) {
        files[file].shp.close();
    }
    print("Done");
}

