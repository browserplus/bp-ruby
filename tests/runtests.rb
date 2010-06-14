#!/usr/bin/env ruby

require File.join(File.dirname(File.dirname(File.expand_path(__FILE__))),
                  'external/dist/share/service_testing/bp_service_runner.rb')
require 'uri'
require 'test/unit'
require 'open-uri'

class TestFileAccess < Test::Unit::TestCase
  def setup
    @interpService = "../build/RubyInterpreter"
  end
  
  def teardown
  end

  def test_file_checksum
    BrowserPlus.run("FileChecksum", @interpService) { |s|
      curDir = File.dirname(__FILE__)
      textfile_path = File.expand_path(File.join(curDir, "services.txt"))
      # XXX: service runner needs to grow up here.
      textfile_uri = (( textfile_path[0] == "/") ? "file://" : "file:///" ) + textfile_path
      assert_equal "babc871bf6893c8313686e31cb87816a",  s.md5({:file => textfile_uri})
    }
  end

  def test_basic_service
    BrowserPlus.run("BasicService", @interpService) { |s|
      require 'pp'
      r = s.hello({:who => 'lloyd'}) { |o|
        assert_equal o['callback'], 1
        assert_equal o['args'], "Hi there lloyd"
      }
      assert_equal r, "hello lloyd"

      # now for syntax error
      assert_raise(RuntimeError) { s.syntax }
    }
  end

  # basic test of built in extensions
  def test_sha1
    BrowserPlus.run("SHA1", @interpService) { |s|
      require 'digest/sha1'
      assert_equal s.sha1, Digest::SHA1.hexdigest("hello world")
    }
  end

  # slightly deeper test of built in extensions, generate
  # some keypairs!
  def test_crypto
    BrowserPlus.run("Crypto", @interpService) { |s|
      assert_equal 342, s.generate.length
      assert_equal 342, s.generate.length
      assert_equal 342, s.generate.length
      assert_equal 342, s.generate.length
      assert_equal 342, s.generate.length
      assert_equal s.listKeys.length, 5
    }
  end

  # A junk ruby file
  def test_syntax_error
    assert_raise(RuntimeError) do 
      BrowserPlus.run("SyntaxError", @interpService) { |s| }
    end
  end

  # A bad type defined within the ruby file - (NOTE: really wish we could
  # test that there's verbose and useful information in the log output)
  def test_bad_type
    assert_raise(RuntimeError) do 
      BrowserPlus.run("BadType", @interpService) { |s| }
    end
  end

  # A bad type defined within the ruby file - (NOTE: really wish we could
  # test that there's verbose and useful information in the log output)
  def test_require_stmt
    BrowserPlus.run("RequireTest", @interpService) { |s|
      assert_equal s.yo, "a string"
    }
  end
end
