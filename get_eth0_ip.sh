#!/bin/sh
if `ethtool eth0 2>/dev/null|grep -q "Link detected: yes"`
then
  ip=`ip -4 addr show eth0 | grep -oP '(?<=inet\s)\d+(\.\d+){3}'`
  echo $ip
else
  echo ""
fi
