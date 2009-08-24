require "extsrc/BitShifter"

class Shifty
  bp_version "1.0.0"
  bp_doc "A ruby service testing the embedding of native extensions"

  def left(trans, args)
    bs = BitShifter.new
    trans.complete(bs.left args[:num])
  end

  bp_doc :left, "left shift an integer one bit
                 <num: integer> what to rotate left"

  def right(trans, args)
    bs = BitShifter.new
    trans.complete(bs.right args[:num])
  end

  bp_doc :right, "right shift an integer one bit
                 <num: integer> what to rotate right"
end
