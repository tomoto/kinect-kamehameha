kinect-kamehameha V1.0b
Author: Tomoto S. Washio
Release Date: 2013/1/13
Project URL: http://code.google.com/p/kinect-kamehameha/

0. これは何？

「かめはめ波」のようなエネルギー波を撃てるKinect Hackです。

1. 前提条件

(1) Kinect (Microsoft), Xtion Live Pro (ASUS), PrimeSensor
    (PrimeSense)等のPrimeSense社製3Dセンサーが必要です。
(2) サポートOSはセンサーによって異なります。
(2.1) Kinectの場合
    - 公式にはWindows 7以降、Kinect SDKがサポートされているWindows OS
      (Win7 Pro 64bitで確認済み)
    - 非公式だがWindows XPでも動作 (WinXP Pro SP3 32bitで確認済み)
    - 非公式だがLinuxでも動作する可能性あり (未確認)
(2.2) その他PrimeSense社製センサーの場合
    - Windows XP以降 (WinXP Pro SP3 32bitとWin7 Pro 64bitで確認済み)
    - Linux (Ubuntu 12.10で確認済み)
(3) それなりに早いCPUとGPUが必要だと思います。
    - どこが最低ラインかはわかりません。
    - 少なくともGPUはOpenGL 2.x以上、プログラマブルシェーダ、
      VAOをサポートしている必要があります。
    - Core i7 2600とGeForce GTX 285で快適に動いています。
    動作報告のあった構成が下記のページにあるので参考にしてください。
    http://code.google.com/p/kinect-kamehameha/wiki/WorkingConfigurations

2. インストール

2.1 Kinect SDK版

- Windows 7以降+Kinect用。
  Kinect SDKサポート環境があるKinectユーザにとっては最も気楽な版です。

- Kinect SDKをダウンロード、インストールしてください。
  http://www.kinectforwindows.org

  その後、本ソフトウェアをを好きなところに展開してください。

2.2 OpenNI版

- Windows+PrimeSenseセンサー一般用。Kinectも使えます[*1]。
  インストールが煩わしいのが欠点です。近い将来後述のOpenNI2版に置き換わる予定です。

- OpenNIと関連ソフトウェア一式をダウンロード、インストールしてください。
  複雑なので次のページを参考にしてください。
  http://code.google.com/p/kinect-ultra/wiki/OpenNI_Downloads

  その後、本ソフトウェアをを好きなところに展開してください。

2.3 OpenNI2(Beta)版

- Windows+PrimeSenseセンサー一般用。Kinectも使えます[*2]。
  従来のOpenNI版に比べてセットアップが簡単になっています。
  ただしアーカイブが大きくまだBetaなのが欠点と言えば欠点かもしれません。

- 本ソフトウェアをを好きなところに展開するだけです。
  Kinectを使うには2.1と同様Kinect SDKをインストールしてください。

2.4 Linux版

- Linux+PrimeSenseセンサー一般用。公式にはKinectは使えません[*2]。
  OpenNI2(Beta)ベースで実装されています。
  Ubuntu 12.10以外で動くかわかりません。
  やってみたい方は御自身で試行錯誤してみてください。

- 本ソフトウェアを好きなところに展開し、install.shをsudoしてください。

[*1] 非公式ですがKinect SDKを入れられないWindows XPでもKinectを使えます。
[*2] Kinect SDKを入れられないOSでKinectを使う非公式の方法があるようですが
     やったことはありません。ヒント: FreenectDriver

3. 使い方

(1) kinect-kamehameha.exeを実行するだけです。
    - 作業ディレクトリは実行ファイルと同じ場所にしてください。
    - [ESC]キーで終了します。
    - [Enter]キーで画面を最大化します。

(2) 下記でデモが見られます。
    - 日本語
      http://www.youtube.com/watch?v=r0l18SWKd88
    - 英語
      http://www.youtube.com/watch?v=lajP7HO7ZyE

(3) 問題があった場合...
    - 次のページのFAQを参照ください: http://code.google.com/p/kinect-kamehameha/wiki/FAQ_ja
    - 次のページに報告お願いします: http://code.google.com/p/kinect-kamehameha/issues

4. ライセンス

LICENSE.TXTを見てください。

5. ビルド方法

(1) Windows: プロジェクトページからソースコードをダウンロードし、
    build2010/kinect-ultra.slnをVisual C++ 2010で開いてください。
    デバッガを使うには作業ディレクトリを ".." に設定してください。
(2) Linux: プロジェクトページからソースコードをダウンロードし、
    buildLinux/BUILD_linux.TXTに従ってください。

Enjoy!
Tomoto
