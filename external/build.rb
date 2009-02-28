#!/usr/bin/env ruby

require 'rbconfig'
require 'fileutils'
require 'pathname'
require 'open-uri'
require 'digest'
include Config

$pkg="ruby-1.9.1-p0"
$md5="0278610ec3f895ece688de703d99143e"
$tarball = "#{$pkg}.tar.bz2"
$url="ftp://ftp.ruby-lang.org/pub/ruby/1.9/#{$tarball}"

if CONFIG['arch'] =~ /mswin/
    $platform = "Windows"
else
    $platform = "Darwin"
end

topDir = File.dirname(File.expand_path(__FILE__))
pkgDir = File.join(topDir, $pkg)
buildDir = File.join(topDir, "ruby_build")

puts "removing previous build artifacts..."
FileUtils.rm_rf(pkgDir)
FileUtils.rm_f("#{$pkg}.tar")
FileUtils.rm_rf(buildDir)

if !File.exist?($tarball)
    puts "fetching tarball from #{$url}"
    perms = $platform == "Windows" ? "wb" : "w"
    totalSize = 0
    lastPercent = 0
    interval = 5
    f = File.new($tarball, perms)
    f.write(open($url,
                 :content_length_proc => lambda {|t|
                     if (t && t > 0)
                         totalSize = t
                         STDOUT.printf("expect %d bytes, percent downloaded: ",
                                       totalSize)
                         STDOUT.flush
                     else 
                         STDOUT.print("unknown size to download: ")
                     end
                 },
                 :progress_proc => lambda {|s|
                     if (totalSize > 0)
                         percent = ((s.to_f / totalSize) * 100).to_i
                         if (percent/interval > lastPercent/interval)
                             lastPercent = percent
                             STDOUT.printf("%d ", percent)
                             STDOUT.printf("\n") if (percent == 100)
                         end
                     else
                         STDOUT.printf(".")
                     end
                     STDOUT.flush
                 }).read)
    f.close()
    s = File.size($tarball)
    if (s == 0 || (totalSize > 0 && s != totalSize))
        puts "download failed"
        FileUtils.rm_f($tarball)
        exit 1
    end
end

# now let's check the md5 sum
calculated_md5 = Digest::MD5.hexdigest(File.open($tarball, "rb") {
                                         |f| f.read
                                       })
if calculated_md5 != $md5
  puts "md5 mismatch, tarball is bogus, delete and retry"
  puts "(got #{calculated_md5}, wanted #{$md5})"
  exit 1
else
  puts "md5 validated! (#{calculated_md5} == #{$md5})"
end
