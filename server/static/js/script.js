$(function() {
  $('input#query').on('keyup', checkChange(this));
  function checkChange(e) {
    var v = $(e).find('input#query').val();
    var old = v;
    return function() {
      v = $(e).find('input#query').val();
      if (old != v) {
        old = v;
        changeEventHandler(v);
      }
    };
  }
  function changeEventHandler(value) {
    $.getJSON('search/' + value, null, function(data) {
      var items = [];
      items.push('<thead>');
      items.push('<tr><th>出現数</th><th>図書名</th><th>著者名</th><th>-</th></tr>');
      items.push('</thead>');
      items.push('<tbody>');
      $.each(data.result, function(index, value) {
        items.push('<tr><td>' + value.count + '</td><td>' + value.title + '</td><td>' + value.author + '</td><td><button class="btn verbose">詳細</button></td></tr>');
      });
      items.push('</tbody>');
      $('table#result').html(items.join(''));
      $('button.verbose').on('click', function() {
        var k = $('button.verbose').index(this);
        var word = $('input#query').val();
        $('table#result tbody tr').each(function(index, value) {
          if(index == k) {
            $(this).addClass('info');
          }
          else {
            $(this).removeClass('info');
          }
        });
        var count = parseInt($('table#result tbody').find('tr:eq(' + k + ')').find('td:eq(0)').text());
        var get = Math.min(count, 20)
        var requests = []
        for(var i = 0; i < get; ++i) {
          requests.push($.get('description/' + k + '/' + i + '/' + word));
        }
        $.when.apply($, requests).done(function() {
          if (requests.length == 1) {
            var data = arguments[0];
            $('table#result2').html('<tr><td>' + data + '</td></tr>');
          }
          else {
            var items = []
            for(var i = 0; i < arguments.length; ++i) {
              var data = arguments[i][0];
              items.push('<tr><td>' + data + '</td></tr>');
            }
            $('table#result2').html(items.join(''));
          }
          var regexp = new RegExp(word, 'g');
          $('table#result2 tr').each(function() {
            $(this).html($(this).html().replace(regexp, '<span style="font-weight: bold; color: #fff; background-color: #333;">' + word + '</span>'));
          });
        });
      });
    });
  }
});
