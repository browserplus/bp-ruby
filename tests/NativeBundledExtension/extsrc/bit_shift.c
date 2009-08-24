/**
 * A "hello world" native extension for the purposes of testing bundled
 * native extensions inside ruby services
 */

#include "ruby.h"

static VALUE t_left(VALUE self, VALUE obj)
{
    VALUE rv = Qnil;
    
    if (TYPE(obj) != T_FIXNUM) {
        rb_raise(rb_eArgError, "wrong argument type");
    } else {
        rv = INT2NUM(NUM2INT(obj) << 1);
    }

    return rv;
}

static VALUE t_right(VALUE self, VALUE obj)
{
    VALUE rv = Qnil;
    
    if (TYPE(obj) != T_FIXNUM) {
        rb_raise(rb_eArgError, "wrong argument type");
    } else {
        rv = INT2NUM(NUM2INT(obj) >> 1);
    }

    return rv;
}

void Init_BitShifter() 
{
    VALUE cBitShifter = rb_define_class("BitShifter", rb_cObject);
    rb_define_method(cBitShifter, "left", t_left, 1);
    rb_define_method(cBitShifter, "right", t_right, 1);
}
