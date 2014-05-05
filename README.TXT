
-------------------------------------------------------------------------------

   meskkdic 1.1.0

-------------------------------------------------------------------------------

SKK辞書をマージしたり差分を取るコマンドラインプログラムです。
skkdic-expr2から共通集合の機能を省いた作りとなっています。

meskkdic.exe

   EUC-JIS-2004(改行LF)やUTF-8(BOMなし、改行LF)なSKK辞書をマージしたり差分を取る
   コマンドラインプログラムです。
   実行の際は、ファイルの文字コードを揃えて下さい。

meskkdicw.exe

   UTF-16(LE、BOMあり、改行CR+LF)なSKK辞書をマージしたり差分を取る
   コマンドラインプログラムです。

実行方法

   meskkdic.exe <input file 1> [[+-] <input file 2> ...] <output file>

使用例

 1) マージして結果を出力
   muskkdic.exe SKK-JISYO.L + SKK-JISYO.JIS3_4 + SKK-JISYO.JIS2004 myskkdic.txt

 2) 差分を取って結果を出力
   muskkdic.exe myskkdic_old.txt - wrong.txt myskkdic_new.txt

出力ファイルはSKK辞書形式にソートされます。
マージのとき同じ候補に異なる注釈があった場合は注釈をカンマで区切って連結します。
送りありエントリの角括弧のブロックは無視されます。



-------------------------------------------------------------------------------

   LICENSE

-------------------------------------------------------------------------------

meskkdic

The MIT License

Copyright (c) 2012-2014 SASAKI Nobuyuki

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

-------------------------------------------------------------------------------
