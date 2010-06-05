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

  def test_basic_service
    BrowserPlus.run("BasicService", "../build/RubyInterpreter") { |s|

    }
  end
end
