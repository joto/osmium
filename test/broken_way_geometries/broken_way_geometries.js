
shapefile('ways').
    filename('tmp/ways').
    type(LINE).
    column('id',  INTEGER, 10).
    column('test', STRING, 100).
    column('note', STRING, 100);

way('test').
    output('ways').
        attr('test').
        attr('note');

