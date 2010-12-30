/* count.js (single pass) */

var tags_counter = {};
var keys_counter = {};

Osmium.Callbacks.node = function() {
    for (key in this.tags) {

        if (keys_counter[key]) {
            keys_counter[key]++;
        } else {
            keys_counter[key] = 1;
        }

        var value = this.tags[key];
        var t = key + '\t' + value;
        if (tags_counter[t]) {
            tags_counter[t]++;
        } else {
            tags_counter[t] = 1;
        }

    }
}

Osmium.Callbacks.init = function() {
    print('Start!');
}

Osmium.Callbacks.end = function() {
    var out_keys_nodes = Osmium.Output.CSV.open('stats-keys-nodes.csv');
    for (key in keys_counter) {
        out_keys_nodes.print(key, keys_counter[key]);
    }
    out_keys_nodes.close();

    var out_tags_nodes = Osmium.Output.CSV.open('stats-tags-nodes.csv');
    for (tag in tags_counter) {
        out_tags_nodes.print(tag, tags_counter[tag]);
    }
    out_tags_nodes.close();

    print('End!');
}

