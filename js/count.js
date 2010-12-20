
/* osmm.js */

var tags_hash = new Object;
var keys      = new Object;

function cb_node() {
//    print(this.lon);
    for (key in this.tags) {
        var value = this.tags[key];
        var t = key + '\t' + value;
        if (tags_hash[t]) {
            tags_hash[t]++;
        } else {
            tags_hash[t] = 1;
        }
        if (keys[key]) {
            keys[key]++;
        } else {
            keys[key] = 1;
        }
    }
}

function cb_way() {
}

function cb_relation() {
}

function cb_init() {
    print('Start!');
}

function cb_end() {
    var out_keys_nodes = Osmium.Output.CSV.open("stats-keys-nodes.tbf");
    for (key in keys) {
        out_keys_nodes.print(key, keys[key]);
    }
    out_keys_nodes.close();
    var out_tags_nodes = Osmium.Output.CSV.open("stats-tags-nodes.tbf");
    for (tag in tags_hash) {
        out_tags_nodes.print(tag, tags_hash[tag]);
    }
    out_tags_nodes.close();
    print('End!');
}

Osmium.Callbacks.init     = cb_init;
Osmium.Callbacks.node     = cb_node;
//Osmium.Callbacks.way      = cb_way;
//Osmium.Callbacks.relation = cb_relation;
Osmium.Callbacks.end      = cb_end;

