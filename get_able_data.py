#!/usr/bin/python3
import sys
import urllib.request
import urllib.error

fr24Data = "/mem/fr24_data"
ableLiveUrl = "/home/pi/able-live.uri"

with open(ableLiveUrl) as inf:
    ableLive = inf.read()

try:
    with open(fr24Data) as inf:
        data = inf.read(35)

    if len(data) < 35:
        print("No data yet")
        sys.exit(0)
except OSError as e:
    print(f"File not found looking for {fr24Data}")
    sys.exit(0)

# Overwrite fr24 data with PilotAware data
try:
    paData = urllib.request.urlopen(ableLive).read(35).decode("utf-8")

    if len(paData) < 35:
        print(data)
        sys.exit(0)

    dataArr = list(data)
    for i in range(12):
        pos = i * 3
        if paData[pos] != "-":
            dataArr[pos] = paData[pos]
            dataArr[pos+1] = paData[pos+1]

    print("".join(dataArr))
except urllib.error.HTTPError as e:
    print(data)
except urllib.error.URLError as e:
    print(data)
