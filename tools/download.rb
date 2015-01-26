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

def get_xhtml_url(card_url)
  dom = Nokogiri::HTML(open(card_url).read())
  dom.xpath('//a').each do |a_tag|
    if a_tag.inner_text == 'いますぐXHTML版で読む'
      # 全くの間違い
      baseurl = card_url.split('/')[0 .. -2].join('/')
      return baseurl + a_tag['href'][1 .. -1]
    end
  end
end

def main
  li = get_book_list()
  sleep 2
  failure = []
  li.each do |card_url|
    puts "card_url = #{card_url}"
    xhtml_url = get_xhtml_url(card_url)
    if xhtml_url == "0"
      failure.push card_url
      next
    end
    sleep 2
    puts "xhtml_url = #{xhtml_url}"
    `curl -O #{xhtml_url}`
    sleep 2
  end
  p failure
end

if __FILE__ == $0
  main
end
