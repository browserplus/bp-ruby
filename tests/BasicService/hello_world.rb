# a rough sketch of how a service under ruby engine 5.x.x may look,
# 
# we'll need some of why's goodness to get here:
# http://github.com/why/fakeplus/blob/master/stdlib/bp_hacks.rb
#
# design credit? why, the lucky stiff, of course.

class Test
  bp_version "1.0.0"
  bp_doc "Hello world service/corelet thingy."

  def hello(code)
    "hello world"
  end

  bp_doc :hello, 
    "return the string, 'hello world'.  original, eh?"
end

# and this should be optional?!
RubyServiceDefinition = Test.to_service_def
