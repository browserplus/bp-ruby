#!/usr/bin/env ruby

require 'rbconfig'
require 'fileutils'
require 'pathname'
include Config

TOPDIR = File.dirname(File.expand_path(__FILE__))

require File.join(TOPDIR, "bakery/ports/bakery")

$order = {
  :output_dir => File.join(TOPDIR, "dist"),
  :packages => [
                "ruby19",
                "service_testing"
               ],
  :verbose => true
}

b = Bakery.new $order

# let's check the bakery state quickly
s = b.check
issues = s[:info].length + s[:warn].length +  s[:error].length
puts "Bakery consistency check complete (#{issues} interesting issues found)"
s.each { |k,v| v.each { |msg| puts "#{k.to_s.upcase}: #{msg}" } }
raise "refusing to build bakery, in an inconsistent state" if s[:error].length > 0
b.build
