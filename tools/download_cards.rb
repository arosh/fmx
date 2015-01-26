# coding: UTF-8
require 'nokogiri'
require 'open-uri'

def get_book_list
  ranking_url = 'http://www.aozora.gr.jp/access_ranking/2013_xhtml.html'
  dom = Nokogiri::HTML(open(ranking_url).read())
  ret = []
  dom.xpath('//tr')[1 .. -1].each do |tr_tag|
    ret.push(tr_tag.xpath('td/a')[0]['href'])
  end
  return ret
end

def main
  li = get_book_list()
  sleep 2
  failure = []
  li.each do |card_url|
    puts "card_url = #{card_url}"
    `curl -O #{card_url}`
    sleep 2
  end
  p failure
end

if __FILE__ == $0
  main
end
