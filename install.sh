if [ "$EUID" -ne 0 ]
  then echo "I need root privileges!"
  exit
fi

make
cp ./rd_watcher /usr/bin/

echo "
[Unit]
Description=Block Rubber Ducky Attacks

[Service]
Type=simple
ExecStart=/usr/bin/rd_watcher
TimeoutSec=30
Restart=on-failure
RestartSec=30
StartLimitInterval=350
StartLimitBurst=10
KillMode=process

[Install]
WantedBy=multi-user.target
" > /etc/systemd/system/rdWatcher.service 
sudo systemctl enable --now rdWatcher.service
