# A little ruby library which can allow client code to programattically drive service runner.
# Great for unit tests!

# if we're talkin' ruby 1.9, we'll use built in json, otherwise use
# the pure ruby library sittin' here
$:.push(File.dirname(__FILE__))
begin
  require 'json'
rescue LoadError
  require "json/json.rb"
end

module BrowserPlus
  module ProcessController
    def getmsg(pio, timeo, lookFor = false)
      j = nil
      @outputbuffer = String.new if (@outputbuffer == nil)
      while true
        begin
          # now peel off the first json map, taking advantage of the fact that
          # ServiceRunner inserts newlines
          m = @outputbuffer.match(/^([^\n]+)\n(.*)$/)
          if m != nil
            # regex pulls off that \n, so add it back!
            @outputbuffer = "#{m[2]}\n";
            j = JSON.parse(m[1])
            return j
          end
        rescue
        end
        break if (nil == select( [ pio ], nil, nil, timeo ))  
        @outputbuffer += pio.sysread(1024) 
      end
      nil
    end
  end

  class Service
    def initialize path, provider = nil
      sr = findServiceRunner
      raise "can't execute ServiceRunner: #{sr}" if !File.executable? sr
      cmd = ""
      if provider != nil
        cmd = "#{sr} -slave -providerPath #{provider} #{path}"
      else
        cmd = "#{sr} -slave #{path}"
      end
      @srp = IO.popen(cmd, "w+")
      i = getmsg(@srp, 0.5)
      raise i['msg'] if i && i['type'] == 'error' && i['msg']
      raise "couldn't initialize" if i['msg'] !~ /service initialized/
      @instance = nil
    end

    # allocate a new instance
    def allocate
      @srp.syswrite "allocate\n"
      i = getmsg(@srp, 2.0)
      raise "couldn't allocate" if !i.has_key?('msg')
      num = i['msg']
      Instance.new(@srp, num)
    end

    # invoke a function on an automatically allocated instance of the service
    def invoke f, a, &cb
      @instance = allocate if @instance == nil
      @instance.invoke f, a, &cb
    end

    def method_missing func, *args, &b
      invoke func, args[0], &b
    end

    def shutdown
      if @instance != nil
        @instance.destroy
        @instance = nil
      end
      @srp.close
      @srp = nil 
    end

    private


    # attempt to find the ServiceRunner binary, a part of the BrowserPlus
    # SDK. (http://browserplus.yahoo.com)
    def findServiceRunner
      # first, try relative to this repo 
      srBase = File.join(File.dirname(__FILE__), "..", "..", "bpsdk", "bin")
      candidates = [
                    File.join(srBase, "ServiceRunner.exe"),
                    File.join(srBase, "ServiceRunner"),             
                   ]
      
      # now use BPSDK_PATH env var if present
      if ENV.has_key? 'BPSDK_PATH'
        candidates.push File.join(ENV['BPSDK_PATH'], "bin", "ServiceRunner.exe")
        candidates.push File.join(ENV['BPSDK_PATH'], "bin", "ServiceRunner")
      end
      
      if ENV.has_key? 'SERVICERUNNER_PATH'
        candidates.push(ENV['SERVICERUNNER_PATH'])
      end

      candidates.each { |p|
        return p if File.executable? p
      }
      nil
    end

    include ProcessController
  end
  
  class Instance
    # private!!
    def initialize p, n
      @iid = n
      @srp = p
    end

    def invoke func, args, &cb
      args = JSON.generate(args).gsub("'", "\\'")
      cmd = "inv #{func.to_s}"
      cmd += " '#{args}'" if args != "null"
      cmd += "\n"
      # always select the current instance
      @srp.syswrite "select #{@iid}\n"
      @srp.syswrite cmd
      while i = getmsg(@srp, 4.0)
        # skip info messages
        next if i['type'] == "info"
        # invoke passed in block for callbacks?
        if i['type'] == "callback"        
          cb.call(i['msg']) if cb != nil
          next
        end
        break
      end
      raise i['msg'] if i && i['type'] == 'error' && i['msg']
      raise "invocation failure" if i == nil || i['type'] != 'results'
      i['msg']
    end

    def destroy
      @srp.syswrite "destroy #{@iid}\n"
    end

    def method_missing func, *args, &cb
      invoke func, args[0], cb
    end
    private
    include ProcessController
  end

  def BrowserPlus.run path, provider = nil, &block
    s = BrowserPlus::Service.new(path, provider)
    block.call(s)
    s.shutdown
  end
end
