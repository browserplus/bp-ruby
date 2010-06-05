#!/usr/bin/env ruby

require File.join(File.dirname(__FILE__), 'bp_service_runner')
require 'uri'
require 'test/unit'
require 'open-uri'

class TestFileAccess < Test::Unit::TestCase
  def setup
  end
  
  def teardown
  end

  def test_file_checksum
    BrowserPlus.run("FileChecksum", "../build/RubyInterpreter") { |s|
      curDir = File.dirname(__FILE__)
      textfile_path = File.expand_path(File.join(curDir, "services.txt"))
      textfile_uri = (( textfile_path[0] == "/") ? "file://" : "file:///" ) + textfile_path
      assert_equal "babc871bf6893c8313686e31cb87816a",  s.md5({:file => textfile_uri})
    }
  end
end
