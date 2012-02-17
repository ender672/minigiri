require "minigiri"

buf = File.read 'sample.xml'

def inspect node
  node.map do |n|
    inspect n
  end
end

loop do
  doc = Document.read_memory(buf)
  inspect doc
end
