projects="/data/share/projects"

home="/home/pi/able-display"
project="$projects/able-display"

sudo rm -rf $project.old >/dev/null 2>/dev/null
sudo mv $project $project.old
sudo cp -rp $home $project
