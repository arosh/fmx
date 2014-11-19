# coding: UTF-8
require 'nokogiri'

def main
  Dir.glob('bunko/*.html').each do |ifname|
    dom = nil
    File.open(ifname, 'r:Shift_JIS') do |f|
      dom = Nokogiri::HTML(f.read)
    end
    dom.xpath('//rp').remove
    dom.xpath('//rt').remove
    dom = dom.xpath('//div[@class="main_text"]')
    text = dom.inner_text.each_line.map{|s| s.chomp.gsub('　', ' ').strip }.select{|s| s != ''}.join(' ')
    ofname = File.dirname(ifname) + '/' + File.basename(ifname, '.*') + '.txt'
    File.open(ofname, 'w:UTF-8') do |f|
      f.write(text.encode('UTF-8'))
    end
  end
end

if __FILE__ == $0
  main
end
