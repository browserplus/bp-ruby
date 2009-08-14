#!/usr/bin/env ruby

require 'rbconfig'
require 'fileutils'
require 'pathname'
require 'open-uri'
require 'digest'
include Config

$pkg="ruby-1.9.1-p243"
$md5="66d4f8403d13623051091347764881a0"
$tarball = "#{$pkg}.tar.bz2"
$url="http://ftp.ruby-lang.org/pub/ruby/#{$tarball}"

if CONFIG['arch'] =~ /mswin/
    $platform = "Windows"
else
    $platform = "Darwin"
end

topDir = File.dirname(File.expand_path(__FILE__))
pkgDir = File.join(topDir, $pkg)
buildDir = File.join(topDir, "ruby_build_output")

if File.exist? buildDir
  puts "it looks like ruby is already built, sheepishly refusing to "
  puts "blow away your build output directory (#{buildDir})"
  puts "sometimes the best thing to do, is nothing."
  exit 0
end

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

# unpack the bugger
puts "unpacking tarball..."
if $platform == "Windows"
  throw "oopsie, implement me please"
#    system("#{topDir}\\..\\Windows\\bin\\7z.exe x #{tarball}")
#    system("#{topDir}\\..\\Windows\\bin\\7z.exe x #{pkg}.tar")
#    FileUtils.rm_f("#{pkg}.tar")
else
  system("tar xjf #{$tarball}")

  # configure & build
  Dir.chdir(pkgDir) do 
    puts "configuring ruby..."

    # we want these bits to work on tiger and leopard regardless of
    # where they're 
    ENV['CFLAGS'] = '-mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk'
    ENV['LDFLAGS'] = '-isysroot /Developer/SDKs/MacOSX10.4u.sdk'

    # now configure...
    system("./configure --prefix=#{buildDir} --enable-shared --disable-install-doc")

    # make & install locally (see configure --prefix arg)
    system("make")
    system("make install")
    system("strip -x #{buildDir}/lib/*.dylib")
  end
end
