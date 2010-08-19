#
# A simple service with a bogus type in its description
#

class BadType
  bp_version "1.0.0"
  bp_doc "A hello world test service for BrowserPlus."

  def hello(trans, args)
    args[:cb].invoke("Hi there #{args[:who]}") if args.has_key? :cb
    trans.complete("hello #{args[:who]}")
  end

  bp_doc :hello, "return the string, 'hello world'.  original, eh?
                  <who: string> who to say hello to.
                  [cb: bogus] a callback to invoke (with a bogus type name)"
end
