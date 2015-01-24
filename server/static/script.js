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
      items.push('<tr><th>ヒット数</th><th>図書名</th><th>著者名</th></tr>');
      $.each(data.result, function(index, value) {
        items.push('<tr><td>' + value.count + '</td><td>' + value.title + '</td><td>' + value.author + '</td></tr>');
      });
      $('table#tablearea').html(items.join(''));
    });
  }
});
