# Ecole42Paris_ft_shield
**_making simple trojan_**

### ⚠️ **WARNING** ⚠️
This project is for educational use only in a sandboxed environment such as a virtual machine.
Do NOT deploy this on any public-facing or production machine.
Unauthorized port opening or creating persistent background processes may be considered malicious outside of your learning setup.
---

### 📦 short brief how to set project before compile
- Make sure you are running in a **VM**  
  (or else it will open your own port `4242` publicly)
- Make sure you are running as **root**

---

### ✅ how to know if it is working (after executing ft_shield)
- `$ systemctl status ft_shield` : service must be running
- `/bin/ft_shield` : file must be present
- `/etc/systemd/system/ft_shield.service` : file must be present
- `/etc/init.d/ft_shield` : file must be present (if using Debian 7 or below - sysvinit)

---

### ⚙️ what does ft_shield do?
- `ft_shield` is a minimal background process designed to mimic the behavior of a basic trojan for educational purposes.
- Once installed and started, it will:
  - Open a port (`4242`) and listen for incoming connections.
  - Ensure persistence across reboots via `systemd`.
  - The binary runs in the background (`/bin/ft_shield`) and avoids termination unless explicitly stopped.
  - If you send a request to the open `4242` port using `nc` (netcat), you will get an interactive shell session.
  - Through this shell, you can execute shell commands, including launching `/bin/sh` with **root privileges**.
```sh
nc [ip adderss] 4242
Keycode: ****
$>
...
```
---

### 🔐 security warning

> This project opens a **remote root shell** on your machine via port `4242`.  
> This means **anyone** who can reach your machine on that port may potentially gain **full root access**, depending on your firewall and network setup.

- **DO NOT** use this outside of a private, isolated, and disposable virtual machine.
- **NEVER** run this on a production system or any network-connected host without full understanding of the consequences.
- Unauthorized deployment or use of such a backdoor mechanism may:
  - **Violate computer crime laws** in many countries.
  - Lead to **disciplinary action** in academic or corporate settings.
  - Result in **permanent system compromise** or data loss.

This tool is strictly for **educational and ethical hacking practice** under controlled environments such as Ecole 42 exercises or offline sandboxes.


---

### 🧹 how to stop or remove ft_shield

```bash
# simply run our shell script
sh reset.sh

# or you can do manually like
# Stop the service
systemctl stop ft_shield

# Disable autostart
systemctl disable ft_shield

# Remove service and binary files
rm -f /etc/systemd/system/ft_shield.service
rm -f /bin/ft_shield

# Reload systemd
systemctl daemon-reexec
systemctl daemon-reload
```
