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
      output = String.new
      # XXX: todo, output handler where multiple messages are combined!
      while nil != select( [ pio ], nil, nil, timeo )  
        output += pio.sysread(1024) 
        begin
          j = JSON.parse(output)
          break
        rescue
        end
      end
      j
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
      raise "couldn't initialize" if !i['msg'] =~ /service initialized/
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
    def invoke f, a = nil
      @instance = allocate if @instance == nil
      @instance.invoke f, a
    end

    def method_missing func, *args
      invoke func, args[0]
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

    def invoke func, args
      args = JSON.generate(args).gsub("'", "\\'")
      cmd = "inv #{func.to_s}"
      cmd += " '#{args}'" if args != "null"
      cmd += "\n"
      # always select the current instance
      @srp.syswrite "select #{@iid}\n"
      @srp.syswrite cmd
      i = getmsg(@srp, 4.0)
      raise "invocation failure" if i['type'] != 'results'
      i['msg']
    end

    def destroy
      @srp.syswrite "destroy #{@iid}\n"
    end

    def method_missing func, *args
      invoke func, args[0]
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
