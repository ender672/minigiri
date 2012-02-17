require "minigiri"

buf = File.read 'sample.xml'

class Node
  include Enumerable
  def each(&block)
    length.times do |x|
      yield self[x]
    end
  end
end

def inspect node
  node.map do |n|
    inspect n
  end
end

loop do
  doc = Document.read_memory(buf)
  inspect doc
end
