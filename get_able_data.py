#!/usr/bin/python3
import sys
import socket
import urllib.request
import urllib.error

fr24Data = "/mem/fr24_data"
ableLiveUrl = "/home/pi/able-live.uri"

with open(ableLiveUrl) as inf:
    ableLive = inf.read()

if socket.gethostname() == "able-display":
    try:
        with open(fr24Data) as inf:
            data = inf.read(35)

        if len(data) < 35:
            print("No data yet")
            sys.exit(0)
    except OSError as e:
        print(f"File not found looking for {fr24Data}")
        sys.exit(0)
else:
    ableDataUrl = "/home/pi/able-display-al.uri"

    with open(ableDataUrl) as inf:
        ableData = inf.read()

    try:
        data = urllib.request.urlopen(ableData).read(35).decode("utf-8")

        if len(data) < 35:
            print("No data yet")
            sys.exit(0)
    except urllib.error.HTTPError as e:
        if e.code == 404:
            print("File not found looking for able-display data")
        else:
            print(f"Tailscale error: {e.code}")
    except urllib.error.URLError as e:
        print(f"Failed to connect to server with error: {e.reason}")


# Overwrite fr24 data with PilotAware data
try:
    paData = urllib.request.urlopen(ableLive).read(35).decode("utf-8")

    if len(paData) < 35:
        print(data)
        sys.exit(0)

    if data[2] != ',':
        print(paData)
        sys.exit(0)

    if paData[2] != ',':
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
