#!/usr/bin/env ruby

require 'rbconfig'
require 'fileutils'
require 'pathname'
include Config

# configure thyself
if CONFIG['arch'] =~ /mswin/
    $platform = "Windows"
elsif CONFIG['arch'] =~ /darwin/
    $platform = "Darwin"
elsif CONFIG['arch'] =~ /linux/
    $platform = "Linux"
else
  raise "unsupported platform: #{CONFIG['arch']}"
end

TOPDIR = File.dirname(File.expand_path(__FILE__))

require File.join(TOPDIR, "bakery/ports/bakery")

$order = {
  :output_dir => File.join(TOPDIR, $platform),
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
