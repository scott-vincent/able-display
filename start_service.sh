sudo systemctl stop able_display

echo Starting ./able_display
sudo cp able_display.service /etc/systemd/system
sudo systemctl enable able_display
sudo systemctl start able_display
