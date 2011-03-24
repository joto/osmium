
shapefile('utf8').
    filename('tmp/utf8').
    type(POINT).
    column('id',   INTEGER, 10).
    column('note', STRING, 100).
    column('utf8', STRING, 8);

node('utf8').
    output('utf8').
        attr('note').
        attr('utf8');

