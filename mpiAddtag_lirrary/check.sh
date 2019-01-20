# yum updateを実行
echo "yum updateを実行します"
echo ""
yum -f update

# 必要なライブラリのインストール
echo "必要なライブラリをインストールします"
echo "wget, kernel-devel installed"
yum install -y wget
yum install -y kernel-devel
