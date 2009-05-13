# Toy crypto service for testing built-in ruby extensions
#

require 'openssl'

class Crypto
  bp_version "1.0.0"
  bp_doc "A tool for generating RSA keypairs on the client"

  def initialize args
    @keys = Array.new 
  end

  def listKeys(trans, args)
    # for this test, we do not persist keys
    trans.complete(@keys.collect {|x| [x.public_key.to_s].pack('m')})
  end

  bp_doc :listKeys, "list public keys associated with the user on this domain"

  def generate(trans, args)
    # generate an RSA key
    rsa = OpenSSL::PKey::RSA.generate(1024)
    @keys.push(rsa)
    # for this test, we do not persist keys
    trans.complete([rsa.public_key.to_s].pack('m'))
  end

  bp_doc :generate, "generate a new RSA public keypair, returning a base64 " +
                    "encoded representation of the public keys"

  def destroy
    puts "destroy called!  thanks for calling my destructor, BrowserPlus"
  end
end
