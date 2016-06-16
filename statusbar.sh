#!/bin/sh
#

battery() {
    if acpi -a |grep off-line > /dev/null; then
        echo "BAT $(acpi -b |awk 'sub(/,/,"") {print $4}') "
    fi
}

clock() {
      date '+%I:%M'
}

cpuload() {
    iostat -c | awk 'BEGIN {sum=0.0f} {sum+=$1} END {print sum}'
}

memram() {
    FREE_DATA=`free -m | grep Mem` 
    CURRENT=`echo $FREE_DATA | cut -f3 -d' '`
    TOTAL=`echo $FREE_DATA | cut -f2 -d' '`
    echo "scale = 2; $CURRENT/$TOTAL*100" | bc
}

wifi() {
    ISUP="$(ip link |grep wlp3s0 |awk '{print $9}')"
    if [ "$ISUP" != "UP" ]; then
        echo -n "WIFI " ; echo -n $ISUP ; echo "! "
    fi
}

while :; do
    buf=" "
    buf="${buf}$(battery)"
    buf="${buf}$(wifi)"
    buf="${buf}CPU[$(cpuload)%] "
    buf="${buf}RAM[$(memram)%] "
    buf="${buf}[$(clock)]"
    xsetroot -name "$buf";
    #echo $buf
    sleep 2 
done
