# Toy crypto service for testing built-in ruby extensions
#

require 'digest/sha1'

class SHA1
  bp_version "1.0.0"
  bp_doc "A simple if digest/ built in extensions"

  def sha1(trans, args)
    trans.complete(Digest::SHA1.hexdigest("hello world"))
  end
  bp_doc :sha1, "calc the sha1 of \"hello world\""
end
