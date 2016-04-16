
-------------------------------------------------------------------------------

   meskkdic 2.2.0

-------------------------------------------------------------------------------

SKK辞書をマージしたり差分を取るコマンドラインプログラムです。

meskkdic.exe

   EUC-JPやEUC-JIS-2004やUTF-8なSKK辞書をマージしたり差分を取るコマンドライン
   プログラムです。実行の際は入力ファイルの文字コードを揃えて下さい。
   入力ファイル
      文字コード : EUC-JP, EUC-JIS-2004, UTF-8(BOMあり/なし)、など
      改行コード : LFまたはCR+LF
   出力ファイル
      文字コード : 入力ファイルと同じ、UTF-8のときはBOMなし
      改行コード : LF

meskkdicw.exe

   UTF-16(LE)なSKK辞書をマージしたり差分を取るコマンドラインプログラムです。
   入力ファイル
      文字コード : UTF-16(LE、BOMあり/なし)
      改行コード : LFまたはCR+LF
   出力ファイル
      文字コード : UTF-16(LE、BOMあり)
      改行コード : CR+LF

実行方法

   meskkdic.exe [-O] <input file 1> [[+-] <input file 2> ...] <output file>

   オプション "-O" があるとき、
     * 出力ファイルはSKK辞書形式にソートされません。
     * 送りありエントリの角括弧のブロックは保持されます。

   オプション "-O" がないとき、
     * 出力ファイルはSKK辞書形式にソートされます。
     * 送りありエントリの角括弧のブロックは無視されます。

使用例

 1) マージして結果を出力 (見出し語のソートあり、送り仮名ブロックを無視)

   muskkdic.exe SKK-JISYO.L + SKK-JISYO.JIS3_4 + SKK-JISYO.JIS2004 myskkdic.txt

 2) マージして結果を出力 (見出し語のソートなし、送り仮名ブロックを保持)

   muskkdic.exe -O jisyo.txt + jisyo2.txt + jisyo3.txt myskkdic.txt

 3) 差分を取って結果を出力 (見出し語のソートあり、送り仮名ブロックを無視)

   muskkdic.exe myskkdic_old.txt - wrong.txt myskkdic_new.txt

 4) 差分を取って結果を出力 (見出し語のソートなし、送り仮名ブロックを保持)

   muskkdic.exe -O myskkdic_old.txt - wrong.txt myskkdic_new.txt


マージのとき同じ候補に異なる注釈があった場合は注釈をカンマで区切って連結します。



-------------------------------------------------------------------------------

   LICENSE

-------------------------------------------------------------------------------

meskkdic

The MIT License

Copyright (c) 2012-2016 SASAKI Nobuyuki

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
