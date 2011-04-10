/* time.js (single pass), can be used for timing tests */

var nodes=0, ways=0, relations=0;

Osmium.Callbacks.node = function(node) {
    nodes += 1;
}

Osmium.Callbacks.way = function(way) {
    ways += 1;
}

Osmium.Callbacks.relation = function(relation) {
    relations += 1;
}

Osmium.Callbacks.end = function() {
    print('nodes: ' + nodes + '  ways: ' + ways + '  relations: ' + relations);
}

