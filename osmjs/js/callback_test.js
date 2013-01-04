// eg:
// osmjs -l sparsetable -2 -m -j callback_test.js <osm_file>

var called_node, called_way, called_relation, called_area;

Osmium.Callbacks.init = function() { print('✔ init'); }
 
Osmium.Callbacks.before_nodes = function() { print('✔ before_nodes'); }
Osmium.Callbacks.node = function() { 
    if (!called_node) {
        print('✔ node');
        called_node = true;
    }
}
Osmium.Callbacks.after_nodes = function() { print('✔ after_nodes'); }
 
Osmium.Callbacks.before_ways = function() { print('✔ before_ways'); }
Osmium.Callbacks.way = function() {
    if (!called_way) {
        print('✔ way');
        called_way = true;
    }
}
Osmium.Callbacks.after_ways = function() { print('✔ after_ways'); }
 
Osmium.Callbacks.before_relations = function() { print('✔ before_relations'); }
Osmium.Callbacks.relation = function() {
    if (!called_relation) {
        print('✔ relation');
        called_relation = true;
    }
}
Osmium.Callbacks.after_relations = function() { print('✔ after_relations'); }
 
Osmium.Callbacks.area = function() {
    if (!called_area) {
        print('✔ area');
        called_area = true;
    }
}

Osmium.Callbacks.end = function() { print('✔ end'); }
