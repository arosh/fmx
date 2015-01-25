# coding: utf-8
from __future__ import division, print_function, unicode_literals
import sys
import flask

import fmx

app = flask.Flask(__name__)

@app.route('/')
def index_route():
    return flask.render_template('index.html')

# converterにはint,str,pathがある
# pathは/を許容する
@app.route('/search/')
def search_null_route():
    return flask.jsonify({ 'result': [] })

@app.route('/search/<path:word>')
def search_route(word):
    return flask.jsonify({ 'result': map(jsonify_line, fmx.search(word.encode('utf_8'), 20)) })

@app.route('/description/<int:k>/<int:i>/<path:word>')
def description_route(k, i, word):
    bytestring = word.encode('utf_8')
    return fmx.description(bytestring, 20, k, i, 300)

def jsonify_line(line):
    count, title, author = line.split('\t')
    return { 'count': count, 'title': title, 'author': author }

if __name__ == '__main__':
    print('init fmx ... ', end='', file=sys.stderr)
    fmx.init()
    print('OK', file=sys.stderr)

    # 外部公開するときはhost='0.0.0.0'
    app.run(debug=True, host='0.0.0.0')
