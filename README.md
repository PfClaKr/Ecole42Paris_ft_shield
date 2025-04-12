# Ecole42Paris_ft_shield

making trojan


### short brief how to set project before compile
- change "/home/s/sh/a.out" to your corresponding path
- make sure you are running as root

### how to know is it working or not? (after executing a.out)
- `$systemctl status ft_shield` : service must be running
- `/bin/ft_shield` : file must be present
- `/etc/systemd/system/ft_shield.service` : file must be present
- `$journalctl -t ft_shield -f` : to see recent activities of this service (debugging)
