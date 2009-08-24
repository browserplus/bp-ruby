#!/usr/bin/env ruby

require 'BitShifter'

x = BitShifter.new
puts "47 << 1 = #{x.left(47)}"
puts "47 >> 1 = #{x.right(47)}"
