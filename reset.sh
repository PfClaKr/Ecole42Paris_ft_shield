#!/bin/sh

# Description:
#
# remove ft_shield service and all related files from system
# that files are include
# - systemd service unit file
# - duplicated binary file


# stop systemd service
systemctl disable ft_shield.service
systemctl stop ft_shield.service

# remove systemd service unit
rm -fr /etc/systemd/system/ft_shield.service

# remove duplicated binary file
rm -fr /bin/ft_shield

echo -e "done"
