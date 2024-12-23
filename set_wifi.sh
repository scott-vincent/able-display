trap "echo ''" INT

netDir="/etc/NetworkManager/system-connections"
netKeep1="$netDir/AerobilityRadio.nmconnection"
netKeep2="$netDir/vincenet-guest.nmconnection"

while [ 1 ]
do
    clear

    echo "Able Display"
    echo ""
    echo "Let's get connected to Wifi"
    echo ""
    echo "Available SSIDs:"
    echo ""
    sudo iw dev wlan0 scan|grep SSID:

    # Flush input
    while [ 1 ]
    do
        read -t 1 flush
        if [ $? -ne 0 ]
        then
            break
        fi
    done

    # Flush connections
    for filename in $netDir/*.nmconnection
    do
        if [ "$filename" != "$netKeep1" -a "$filename" != "$netKeep2" ]
        then
            echo ""
            echo Removing `basename "$filename"`
            sudo rm "$filename" 2>/dev/null
        fi
    done
    sudo nmcli con reload
    echo ""

    read -p "Enter your SSID: " ssid
    if [ "$ssid" = "" ]
    then
        continue
    fi

    if [ "$ssid" = "scott" ]
    then
        break
    fi

    sudo nmcli device wifi connect "$ssid" --ask 2>/dev/null
    if [ $? -eq 0 ]
    then
        echo ""
        echo Wifi connected successfully.
        echo Now remove the ethernet lead. The display will be rebooted in 10 seconds.
        sudo nmcli con reload
        sleep 5
        sudo reboot
    else
        echo ""
        echo Wifi connection failed. Please try again.
        sudo rm "$netDir/$ssid.nmconnection" 2>/dev/null
        sudo nmcli con reload
        sleep 5
    fi
done
trap INT
echo "Hello Scott"
