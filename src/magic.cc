#include <node.h>
#include <node_buffer.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "magic.h"

using namespace v8;
using namespace node;

#define SET_MAGIC_FLAG(t,v) \
    (t)->Set( \
        v8::String::NewSymbol(#v),  \
        v8::Number::New( MAGIC_##v ),                                \
        static_cast<v8::PropertyAttribute>( v8::ReadOnly|v8::DontDelete) \
    )

#define RetIfNotOpen(m) do{\
    if( !m->magic ){ \
        return Throw( Str2Err( "magic does not opened" ) ); \
    }\
}while(0)

#define ObjUnwrap(tmpl,obj) ObjectWrap::Unwrap<tmpl>(obj)
#define Arg2Str(v)          *(String::Utf8Value( (v)->ToString()))
#define Arg2Int(v)          ((v)->IntegerValue())
#define Arg2Uint32(v)       ((v)->Uint32Value())
#define Throw(v)            ThrowException(v)
#define Str2Err(v)          Exception::Error(String::New(v))
#define Str2TypeErr(v)      Exception::TypeError(String::New(v))

// interface
class Magic : public ObjectWrap 
{
    public:
        Magic();
        ~Magic();
        static void Initialize( Handle<Object> target );
        static Handle<Value> GetPath( const Arguments& argv );
    
    private:
        magic_t magic;
        
        static Handle<Value> New( const Arguments& argv );
        
        static Handle<Value> Open( const Arguments& argv );
        static Handle<Value> Close( const Arguments& argv );
        static Handle<Value> Error( const Arguments& argv );
        static Handle<Value> Errno( const Arguments& argv );
        static Handle<Value> Load( const Arguments& argv );
        static Handle<Value> Compile( const Arguments& argv );
        static Handle<Value> Descriptor( const Arguments& argv );
        static Handle<Value> File( const Arguments& argv );
        static Handle<Value> Buffer( const Arguments& argv );
        static Handle<Value> SetFlags( const Arguments& argv );
        static Handle<Value> Check( const Arguments& argv );
        static Handle<Value> List( const Arguments& argv );
};

Magic::Magic(){
    magic = NULL;
};

Magic::~Magic()
{
    if( magic ){
        magic_close( magic );
    }
}

Handle<Value> Magic::GetPath( const Arguments& argv ){
    HandleScope scope;
    return scope.Close( String::New( magic_getpath( NULL, 0 ) ) );
}

Handle<Value> Magic::New( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = new Magic();
    
    m->Wrap( argv.This() );
    
    return scope.Close( argv.This() );
}

Handle<Value> Magic::Open( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    Handle<Value> retval = Undefined();
    int flgs = MAGIC_NONE;
    int argc = argv.Length();
    
    if( argc > 0 )
    {
        int i = 0;
        for(; i < argc; i++ )
        {
            if( !argv[i]->IsUint32() ){
                return Throw( Str2TypeErr( "invalid type of arguments" ) );
            }
            flgs |= Arg2Int( argv[i] );
        }
    }
    
    if( m->magic ){
        magic_close( m->magic );
    }
    
    if( !( m->magic = magic_open( flgs ) ) ){
        retval = Throw( Str2Err( strerror( errno ) ) );
    }
    
    return scope.Close( retval );
}

Handle<Value> Magic::Close( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    
    RetIfNotOpen( m );
    
    magic_close( m->magic );
    m->magic = NULL;
    
    return scope.Close( Undefined() );
}

Handle<Value> Magic::Error( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    const char *str = NULL;
    
    RetIfNotOpen( m );
    
    if( ( str = magic_error( m->magic ) ) ){
        return scope.Close( String::New( str ) );
    }
    return scope.Close( Undefined() );
}

Handle<Value> Magic::Errno( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    int ec = 0;
    
    RetIfNotOpen( m );
    ec = magic_errno( m->magic );
    
    return scope.Close( Integer::New( ec ) );
}

Handle<Value> Magic::Load( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    int argc = argv.Length();
    const char *path = NULL;
    
    RetIfNotOpen( m );

    if( argc )
    {
        if( argv[0]->IsString() ){
            path = Arg2Str( argv[0] );
        }
        else {
            return scope.Close( Throw( Str2TypeErr( "invalid type of arguments" ) ) );
        }
    }
    
    return scope.Close( Integer::New( magic_load( m->magic, path ) ) );
}

Handle<Value> Magic::Compile( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    int argc = argv.Length();
    const char *path = NULL;
    
    RetIfNotOpen( m );
    
    if( argc )
    {
        if( argv[0]->IsString() ){
            path = Arg2Str( argv[0] );
        }
        else {
            return scope.Close( Throw( Str2TypeErr( "invalid type of arguments" ) ) );
        }
    }
    
    return scope.Close( Integer::New( magic_compile( m->magic, path ) ) );
}

Handle<Value> Magic::Descriptor( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    Handle<Value> retval = Undefined();
    int argc = argv.Length();
    
    RetIfNotOpen( m );
    
    if( !argc ){
        retval = Throw( Str2Err( "undefined arguments" ));
    }
    else if( !argv[0]->IsNumber() ){
        retval = Throw(Str2TypeErr( "invalid type of arguments" ) );
    }
    else
    {
        int fd = Arg2Uint32( argv[0] );
        const char *mime = magic_descriptor( m->magic, fd );
        
        if( mime ){
            return scope.Close( String::New( mime ) );
        }
    }
    
    return scope.Close( Undefined() );
}

Handle<Value> Magic::File( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    Handle<Value> retval = Undefined();
    int argc = argv.Length();
    
    RetIfNotOpen( m );
    
    if( !argc ){
        retval = Throw( Str2Err( "undefined arguments" ));
    }
    else if( !argv[0]->IsString() ){
        retval = Throw(Str2TypeErr( "invalid type of arguments" ) );
    }
    else
    {
        Local<String> str = argv[0]->ToString();
        size_t len = str->Length();
        
        if( len )
        {
            const char *mime = magic_file( m->magic, *String::Utf8Value( str ) );
            
            if( mime ){
                return scope.Close( String::New( mime ) );
            }
        }
    }
    
    return scope.Close( Undefined() );
}

Handle<Value> Magic::Buffer( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    Handle<Value> retval = Undefined();
    int argc = argv.Length();
    
    RetIfNotOpen( m );
    
    if( !argc ){
        retval = Throw( Str2Err( "undefined arguments" ));
    }
    else if( !argv[0]->IsString() ){
        retval = Throw(Str2TypeErr( "invalid type of arguments" ) );
    }
    else
    {
        Local<String> str = argv[0]->ToString();
        size_t len = str->Utf8Length();
        
        if( len )
        {
            const char *mime = magic_buffer( m->magic, *String::Utf8Value( str ), len );
            
            if( mime ){
                return scope.Close( String::New( mime ) );
            }
        }
    }
    
    return scope.Close( Undefined() );
}

Handle<Value> Magic::SetFlags( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    Handle<Value> retval = Undefined();
    int flgs = MAGIC_NONE;
    int argc = argv.Length();
    
    RetIfNotOpen( m );
    
    if( argc > 0 )
    {
        int i = 0;
        for(; i < argc; i++ )
        {
            if( !argv[i]->IsUint32() ){
                return Throw( Str2TypeErr( "invalid type of arguments" ) );
            }
            flgs |= Arg2Int( argv[i] );
        }
    }
    
    return scope.Close( Integer::New( magic_setflags( m->magic, flgs ) ) );
}

Handle<Value> Magic::Check( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    int argc = argv.Length();
    const char *path = NULL;
    
    if( argc )
    {
        if( argv[0]->IsString() ){
            path = Arg2Str( argv[0] );
        }
        else {
            return scope.Close( Throw( Str2TypeErr( "invalid type of arguments" ) ) );
        }
    }
    
    return scope.Close( Integer::New( magic_check( m->magic, path ) ) );
}

Handle<Value> Magic::List( const Arguments& argv )
{
    HandleScope scope;
    Magic *m = ObjUnwrap( Magic, argv.This() );
    int argc = argv.Length();
    const char *path = NULL;
    
    RetIfNotOpen( m );
    
    if( argc )
    {
        if( argv[0]->IsString() ){
            path = Arg2Str( argv[0] );
        }
        else {
            return scope.Close( Throw( Str2TypeErr( "invalid type of arguments" ) ) );
        }
    }
    
    return scope.Close( Integer::New( magic_list( m->magic, path ) ) );
}


void Magic::Initialize( Handle<Object> target )
{
    HandleScope scope;
    Local<FunctionTemplate> t = FunctionTemplate::New( New );
    
    t->InstanceTemplate()->SetInternalFieldCount(1);
    t->SetClassName( String::NewSymbol("magic") );
    // set class method
    NODE_SET_METHOD( t, "getpath", GetPath );
    // set constants
    SET_MAGIC_FLAG( t, NONE );
    SET_MAGIC_FLAG( t, DEBUG );
    SET_MAGIC_FLAG( t, SYMLINK );
    SET_MAGIC_FLAG( t, COMPRESS );
    SET_MAGIC_FLAG( t, DEVICES );
    SET_MAGIC_FLAG( t, MIME_TYPE );
    SET_MAGIC_FLAG( t, CONTINUE );
    SET_MAGIC_FLAG( t, CHECK );
    SET_MAGIC_FLAG( t, PRESERVE_ATIME );
    SET_MAGIC_FLAG( t, RAW );
    SET_MAGIC_FLAG( t, ERROR );
    SET_MAGIC_FLAG( t, MIME_ENCODING );
    SET_MAGIC_FLAG( t, MIME );
    SET_MAGIC_FLAG( t, APPLE );
    
    SET_MAGIC_FLAG( t, NO_CHECK_COMPRESS );
    SET_MAGIC_FLAG( t, NO_CHECK_TAR );
    SET_MAGIC_FLAG( t, NO_CHECK_SOFT );
    SET_MAGIC_FLAG( t, NO_CHECK_APPTYPE );
    SET_MAGIC_FLAG( t, NO_CHECK_ELF );
    SET_MAGIC_FLAG( t, NO_CHECK_TEXT );
    SET_MAGIC_FLAG( t, NO_CHECK_CDF );
    SET_MAGIC_FLAG( t, NO_CHECK_TOKENS );
    SET_MAGIC_FLAG( t, NO_CHECK_ENCODING );
    // backwards copatibility(rename)
    SET_MAGIC_FLAG( t, NO_CHECK_ASCII );
    // backwards copatibility(do nothing)
    SET_MAGIC_FLAG( t, NO_CHECK_FORTRAN );
    SET_MAGIC_FLAG( t, NO_CHECK_TROFF );
    
    // set instance method
    NODE_SET_PROTOTYPE_METHOD( t, "open", Open );
    NODE_SET_PROTOTYPE_METHOD( t, "close", Close );
    NODE_SET_PROTOTYPE_METHOD( t, "error", Error );
    NODE_SET_PROTOTYPE_METHOD( t, "errno", Errno );
    NODE_SET_PROTOTYPE_METHOD( t, "load", Load );
    NODE_SET_PROTOTYPE_METHOD( t, "compile", Compile );
    NODE_SET_PROTOTYPE_METHOD( t, "file", File );
    NODE_SET_PROTOTYPE_METHOD( t, "buffer", Buffer );
    NODE_SET_PROTOTYPE_METHOD( t, "descriptor", Descriptor );
    NODE_SET_PROTOTYPE_METHOD( t, "setflags", SetFlags );
    NODE_SET_PROTOTYPE_METHOD( t, "check", Check );
    NODE_SET_PROTOTYPE_METHOD( t, "list", List );
    
    target->Set( String::NewSymbol("magic"), t->GetFunction() );
}

static void init( Handle<Object> target ){
    HandleScope scope;
    Magic::Initialize( target );
}

NODE_MODULE( magic, init );

