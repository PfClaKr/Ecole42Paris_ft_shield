# Ecole42Paris_ft_shield

making trojan


### short brief how to set project before compile
- make sure you are running as root

### how to know is it working or not? (after executing a.out)
- `$systemctl status ft_shield` : service must be running
- `/bin/ft_shield` : file must be present
- `/etc/systemd/system/ft_shield.service` : file must be present
- `/etc/init.d/ft_shield` : file must be present (if you are using Debian 7 and below - sysvinit)
- `$journalctl -t ft_shield -f [-n <num>]` : to see recent activities of this service (debugging)
