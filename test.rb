require "minigiri"

buf = File.read 'sample.xml'

class NodeSet
  include Enumerable
  def each(&block)
    length.times do |x|
      yield self[x]
    end
  end
end

def inspect node
  set = node.children
  set.map do |n|
    inspect n
  end
end

loop do
  doc = Document.read_memory(buf)
  inspect doc
end
