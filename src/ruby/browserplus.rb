#!/usr/bin/env ruby

#
# This beautiful work was originally concieved of by why the lucky stiff:
# http://github.com/why/fakeplus/blob/master/stdlib/bp_hacks.rb
#

$BrowserPlusEntryPointClass = nil

class Array
  def / len
    a = []
    each_with_index do |x, i|
      a << [] if i % len == 0
      a.last << x
    end
    a
  end
end

class Class
  def bp_version v
    @bp_version = v.chomp
  end

  def bp_doc m, desc = nil
    if $BrowserPlusEntryPointClass == nil
      $BrowserPlusEntryPointClass = self
    elsif $BrowserPlusEntryPointClass != self
      throw "multiple entry point classes detected!  you may only have " +
            "a single \"entry point class\" which uses bp_doc functions"
    end
      
    @bp_doc ||= {}
    m, desc = nil, m unless desc
    @bp_doc[m] = parse_bp_doc(m, desc)
  end

  def parse_bp_doc mname, desc
    mname ||= :initialize
    ar = instance_method(mname).arity rescue -1

    doc, *m = desc.split(/^\s+([\[<])(\w+:\s*\w+)[\]>]\s+/)

    m = (m/3).map do |bracket, arg, adoc|
      aname, atype = arg.split(/:\s*/, 2)
      {'name' => aname, 'type' => atype, 'documentation' => adoc,
       'required' => bracket == "<"}
    end.compact
    [doc.strip, m]
  end

# (lloyd) Not implementing wrapping quite yet, as it breaks async function
# returns
#
#  def bp_wrap meth, margs
#    alias_method "bp_#{meth}", meth
#    case meth when 'initialize'
#      define_method meth do |bp_args|
#        args = []
#        send("bp_#{meth}", *args)
#      end
#    else
#      define_method meth do |bp, bp_args|
#        begin
#          args = []
#          if margs
#            args = margs.map { |a| bp_args[a['name']] }
#          end
#          bp.complete(send("bp_#{meth}", *args))
#        rescue Exception => e
#          bp.error('FAIL', "Error: #{e}")
#        end
#      end
#    end
#  end
 
  def to_service_description
    doc, init = @bp_doc[nil]
    hsh =
      {'class' => self.name,
       'name' => self.name,
       'version' => @bp_version,
       'documentation' => doc,
       'functions' => []}
    @bp_doc.each do |k, (v, m)|
      next unless k
      hsh['functions'] << {
        'name' => k.to_s,
        'documentation' => v,
        'arguments' => m
      }
    end
    hsh
  end
end
