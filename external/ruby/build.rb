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
$patches = [ ]

topDir = File.dirname(File.expand_path(__FILE__))
pkgDir = File.join(topDir, $pkg)
buildDir = File.join(topDir, "ruby_build_output")

if CONFIG['arch'] =~ /mswin/
    $platform = "Windows"
    $patches = [ "win32_has_vsnprintf.patch" ]
    $patchProgram = File.join(topDir, "..", "WinTools", "patch.exe")
else
    $platform = "Darwin"
    $patches = [ "snow_leopard_back_compat.patch" ]
    $patchProgram = "patch"
end

if File.exist? buildDir
  puts "it looks like ruby is already built, sheepishly refusing to "
  puts "blow away your build output directory (#{buildDir})"
  puts "sometimes the best thing to do, is nothing."
  exit 0
end

puts "***** removing previous build artifacts..."
FileUtils.rm_rf(pkgDir)
FileUtils.rm_f("#{$pkg}.tar")
FileUtils.rm_rf(buildDir)

if !File.exist?($tarball)
    puts "***** fetching tarball from #{$url}"
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
        puts "***** download failed"
        FileUtils.rm_f($tarball)
        exit 1
    end
end

# now let's check the md5 sum
calculated_md5 = Digest::MD5.hexdigest(File.open($tarball, "rb") {
                                         |f| f.read
                                       })
if calculated_md5 != $md5
  puts "***** md5 mismatch, tarball is bogus, delete and retry"
  puts "***** (got #{calculated_md5}, wanted #{$md5})"
  exit 1
else
  puts "***** md5 validated! (#{calculated_md5})"
end

# unpack the bugger
puts "***** unpacking tarball..."
if $platform == "Windows"
  system("#{topDir}\\..\\WinTools\\7z.exe x #{$tarball}")
  system("#{topDir}\\..\\WinTools\\7z.exe x #{$pkg}.tar")
  FileUtils.rm_f("#{$pkg}.tar")
else
  system("tar xjf #{$tarball}")
end

# patch it up
puts "***** patching"
Dir.chdir(pkgDir) do 
  $patches.each { |p|
	p = File.join("..", p)
    system("#{$patchProgram} -p1 < #{p}")
  }
end

Dir.chdir(pkgDir) do 

  # configure & build
  puts "***** configuring ruby..."
  if $platform == "Windows"
    ENV['RUNTIMEFLAG'] = '-MT'
    system("win32\\configure.bat --prefix=#{buildDir} --disable-install-doc")
  else
    # we want these bits to work on tiger and leopard regardless of
    # where they're 
    ENV['CFLAGS'] = '-mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386'
    ENV['LDFLAGS'] = '-isysroot /Developer/SDKs/MacOSX10.4u.sdk'
    ENV['CC'] = 'gcc-4.0'
    ENV['CXX'] = 'g++-4.0'

    # now configure...
    system("./configure --prefix=#{buildDir} --enable-shared --disable-install-doc")
  end 

  # make & install locally (see configure --prefix arg)
  puts "***** building ruby..."
  if $platform == "Windows"
    system("nmake")
    system("nmake install")
  else
    system("make")
    system("make install")
    system("strip -x #{buildDir}/lib/*.dylib")
  end
end
