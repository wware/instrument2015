#!/bin/sh

# Mostly taken from
# http://www.andrewdotni.ch/blog/2015/02/28/midi-synth-with-raspberry-p/

if [ "$(whoami)" != "root" ]
then
    echo "This script must be run with sudo"
    exit 1
fi

if [ ! -f /home/pi/.1apt_get ]
then
    apt-get update
    apt-get install upstart
    apt-get install -y fluidsynth alsa
    touch /home/pi/.1apt_get
fi

if [ ! -f /home/pi/.2synth_user ]
then
    useradd -G audio synth
    mkdir -p /home/synth
    chown synth /home/synth
    cat >> /etc/sudoers <<EOF
synth ALL=(ALL) NOPASSWD: ALL
EOF
    cat > ~synth/.profile <<EOF
# Run fluidsynth, but this time as a non-interactive server
fluidsynth -is --audio-driver=alsa --gain 3 /usr/share/sounds/sf2/FluidR3_GM.sf2 &

# Give fluidsynth a nice high priority so it gets as much CPU as possible!
sudo renice -n -18 -u synth

# give it plenty of time to boot up
sleep 20

# connect the controller to fluidsynth
aconnect 20:0 128:0
EOF
    chown synth ~synth/.profile
    touch /home/pi/.2synth_user
fi

if [ ! -f /home/pi/.3tty1_conf ]
then
    cat > /etc/init/tty1.conf <<EOF
start on stopped rc RUNLEVEL=[2345] and (
            not-container or
            container CONTAINER=lxc or
            container CONTAINER=lxc-libvirt)

stop on run level [!2345]

respawn
exec /sbin/getty -8 -a synth 38400 tty1
EOF
    touch /home/pi/.3tty1_conf
fi