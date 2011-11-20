/* time.js (single pass), can be used for timing tests */

var nodes=0, ways=0, relations=0;

Osmium.Callbacks.node = function() {
    nodes += 1;
}

Osmium.Callbacks.after_nodes = function() {
}

Osmium.Callbacks.way = function() {
    if (ways == 0) {
        print('first way');
    }
    ways += 1;
}

Osmium.Callbacks.relation = function() {
    if (relations == 0) {
        print('first relation');
    }
    relations += 1;
}

Osmium.Callbacks.end = function() {
    print('nodes: ' + nodes + '  ways: ' + ways + '  relations: ' + relations);
}

