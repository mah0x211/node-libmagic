var util = require('util'),
    fs = require('fs'),
    magic = require('../'),
    obj = new magic();

console.log( 
    util.inspect( 
        magic, 
        { colors: true, depth: 100, showHidden: true }
    )
);

function test_getpath(){
    console.log( 'getpath: ', magic.getpath() );
}

function test_open(){
    obj.open( 
        magic.MIME, 
        magic.NO_CHECK_COMPRESS,
        magic.NO_CHECK_TAR,
        magic.NO_CHECK_APPTYPE,
        magic.NO_CHECK_ELF,
        magic.NO_CHECK_CDF,
        magic.NO_CHECK_TOKENS
    );
}

function test_load(){
    var res = obj.load();
    console.log( 'load: ', res );
    if( res !== 0 ){
        test_error();
    }
}

function test_compile(){
    var res = obj.compile();
    console.log( 'compile: ', res );
    if( res !== 0 ){
        test_error();
    }
}

function test_descriptor()
{
    var fd = fs.openSync('./test.js', 'r' );
    if( fd ){
        console.log( 'descriptor:', fd, '->', obj.descriptor( fd ) );
    }
}

function test_file()
{
    var path = fs.realpathSync( './test.js' );
    console.log( 'file:', path, ' -> ', obj.file( path ) );
}

function test_buffer()
{
    var path = fs.realpathSync( './test.js' ),
        str = fs.readFileSync( path, 'utf8' );
    
    console.log( 'buffer:', path, obj.buffer( str ) );
}

function test_setflags()
{
    console.log( 
        'setflags:', 
        obj.setflags( 
            magic.MIME_TYPE, 
            magic.NO_CHECK_COMPRESS,
            magic.NO_CHECK_TAR,
            magic.NO_CHECK_APPTYPE,
            magic.NO_CHECK_ELF,
            magic.NO_CHECK_CDF,
            magic.NO_CHECK_TOKENS
        )
    );
}

function test_check()
{
    var path = magic.getpath(),
        res = obj.check( path );
    
    console.log( 'check:', path, ' -> ', res );
    if( res !== 0 ){
        test_error();
    }
}

function test_list()
{
    var path = magic.getpath(),
        res = obj.list( path );
    
    console.log( 'list:', path, ' -> ', res );
    if( res !== 0 ){
        test_error();
    }
}

function test_close(){
    obj.close();
}

function test_error(){
    console.log( 'error: ', obj.error() );
    test_errno();
}

function test_errno(){
    console.log( 'errno: ', obj.errno() );
}

test_getpath();
test_open();
test_load();
test_compile();
test_descriptor();
test_file();
test_buffer();
test_setflags();
test_check();
test_list();
test_close();

console.log( 'done' );
