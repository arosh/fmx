# coding: UTF-8
require 'nokogiri'
require 'json'

def main
  File.open('metadata.json', 'w:UTF-8') do |io|
    j = []
    Dir.glob('bunko/*.html').each do |ifname|
      dom = nil
      File.open(ifname, 'r:Shift_JIS') do |f|
        dom = Nokogiri::HTML(f.read)
      end
      title = dom.xpath('//h1[@class="title"]')[0].inner_text
      creator = dom.xpath('//h2[@class="author"]')[0].inner_text
      j.push({ filename: ifname, title: title, creator: creator })
    end
    JSON.dump(j, io)
  end
end

if __FILE__ == $0
  main
end
