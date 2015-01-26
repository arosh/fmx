# coding: UTF-8
require 'nokogiri'
require 'json'

def main
  File.open('metadata.json', 'w:UTF-8') do |io|
    j = []
    failure = []
    ignore = %w{
    bunko/474.txt
    bunko/790.txt
    bunko/825.txt
    bunko/904.txt
    bunko/946.txt
    bunko/974_318.txt
    }
    Dir.glob('cards/*.html').each do |ifname|
      dom = nil
      File.open(ifname, 'r:UTF-8') do |f|
        dom = Nokogiri::HTML(f.read)
      end
      table = dom.xpath('//table[@summary="タイトルデータ"]')[0]
      title = nil
      author = nil
      table.xpath('tr').each do |tr_tag|
        if tr_tag.xpath('td')[0].inner_text == '作品名：'
          if title
            failure << ifname
          end
          title = tr_tag.xpath('td')[1].inner_text
        end
        if tr_tag.xpath('td')[0].inner_text == '著者名：'
          if author
            failure << ifname
          end
          author = tr_tag.xpath('td')[1].inner_text
        end
      end
      book_path = nil
      dom.xpath('//a').each do |a_tag|
        if a_tag.inner_text =~ /いますぐX?HTML版で読む/
          book_path = a_tag['href']
        end
      end
      if title == nil or author == nil or book_path == nil
        failure << ifname
      end

      [title, author, book_path].each do |str|
        str.gsub!('　', ' ')
        str.strip!
      end

      # p [title, author, book_path]
      ofname = 'bunko' + '/' + File.basename(book_path, '.*') + '.txt'
      if ignore.include? ofname
        puts ifname
        next
      end
      j.push({ filename: ofname, title: title, author: author })
    end
    JSON.dump(j, io)
    p failure: failure
  end
end

if __FILE__ == $0
  main
end
