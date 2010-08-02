#
# A simple BrowserPlus service implemented in Ruby that will calculate the
# MD5 sum of a file that a user has selected
#

require 'digest/md5'

class FileChecksum
  bp_version "1.0.10"
  bp_doc "Allows client side MD5 of user selected files." 

  def md5(bp, args)
    begin
      contents = File.open(args[:file].realpath, "rb") { |f| f.read }
      #bp.complete(Digest::MD5.hexdigest(contents))
      #bp.complete(contents)
      bp.complete(contents.length.to_s)
    rescue Exception => err
      bp.error("Error", err.to_s)
    end
  end

  bp_doc :md5, "Generate an md5 checksum of a file.
                <file: path> The file for which to calculate a checksum."
end
