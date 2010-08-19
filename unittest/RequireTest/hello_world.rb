#
# A service that requires another file
#

require "myinclude"

class HelloWorld
  bp_version "1.0.0"
  bp_doc "A hello world test service for BrowserPlus."

  def yo(trans, args)
    trans.complete(MyInclude.getAString)
  end

  bp_doc :yo, "return a string"
end
