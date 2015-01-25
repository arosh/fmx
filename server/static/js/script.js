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
      items.push('<tr><th>ヒット数</th><th>図書名</th><th>著者名</th><th>-</th></tr>');
      $.each(data.result, function(index, value) {
        items.push('<tr><td>' + value.count + '</td><td>' + value.title + '</td><td>' + value.author + '</td><td><button class="btn verbose">詳細</button></td></tr>');
      });
      $('table#tablearea').html(items.join(''));
      $('button.verbose').on('click', function() {
        var k = $('button.verbose').index(this);
        console.log('button index = ' + k);
        var word = $('input#query').val();
        console.log(word);
        console.dir($('table#tablearea').find('tr:eq(' + (k + 1) + ')').html());
        var count = parseInt($('table#tablearea').find('tr:eq(' + (k + 1) + ')').find('td:eq(0)').text());
        console.log(count);
        var get = Math.min(count, 20)
        var items = []
        for(var i = 0; i < get; ++i) {
          $.get('description/' + k + '/' + i + '/' + word).done(function(data) {
            items.push(data);
          });
        }
        console.dir(items);
      });
    });
  }
});
