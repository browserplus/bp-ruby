#!/usr/bin/env ruby

require 'fileutils'
require 'rbconfig'
include Config

if CONFIG['arch'] =~ /mswin/
  rv = system("sed --help > devenv.out 2>&1")    
  FileUtils.rm_f("devenv.out")
  if !rv
    puts "************************************************************"
    puts "sed.exe not in search path, install from (and then RENAME!):"
    puts "http://gnuwin32.sourceforge.net/packages/sed.htm"
    puts "************************************************************"
    exit -1
  end

  rv = system("bison -h > devenv.out 2>&1")    
  FileUtils.rm_f("devenv.out")
  if !rv
    puts "************************************************************"
    puts "bison.exe not in search path, install from (and then RENAME!):"
    puts "http://gnuwin32.sourceforge.net/packages/bison.htm"
    puts "************************************************************"
    exit -1
  end
end

require "bakery/ports/bakery"

$order = {
  :output_dir => File.join(File.dirname(__FILE__), "built"),
  :packages => [ "ruby19" ],
  :verbose => true
}

b = Bakery.new $order
b.build
