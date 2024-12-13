#!/usr/bin/python3
import sys
import urllib.request
import urllib.error

# --------
# Main
# --------

ableLive = sys.argv[1]

try:
    data = urllib.request.urlopen(ableLive).read(35).decode("utf-8")
    print(data)
except urllib.error.HTTPError as e:
    if e.code == 404:
        print("File not found looking for able-live data")
    else:
        print(f"Tailscale error: {e.code}")
except urllib.error.URLError as e:
    print(f"Failed to connect to server with error: {e.reason}")
