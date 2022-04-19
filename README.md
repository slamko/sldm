# Suckless Display Manager for X

- ### **Why?**

There already are some command-line dispay managers across the open source, but the problem with them is that they are ***Display Managers***.
Talks about incorrectness of this name are around since the very beggining of display managers, and the reason for that is they try to mix few different tasks in one thing, which isn't really the unix way. Display managers are nothing more than login managers + session managers. But we already have a beautifull, fully configurable login manager - getty. So this one is simply a session manager for Xorg. To be more specific it is basically an alias manager for Xorg configs and a wrapper around startx. So from perspective of everyday user that generally uses one or two WM's/DE's/Whatever there is no practical need to always run those bloated display managers with cute backgrounds and floating widgets just in order to enter the desktop. All in all, this is intended to help managing sessions without a display manager.  

- ### **Getting started**
Installing dependencies:
```
sudo apt install libncurses5-dev libncursesw5-dev
```
```
sudo pacman -S ncurses
```
```
sudo yum install ncurses-devel
```
```
sudo dnf install ncurses-devel
```

Build:
```
git clone https://github.com/slamko/sldm
cd sldm
sudo make install
```
*Sldm* is designed not to be tied to systemd or SysVinit, it is just a command line tool to be run in tty, so you can simply add something like this to your .bashrc/whatever: 
```
[ $(tty) = "/dev/tty1" ] && sldm
```  

- ### **Usage**
```
sldm add <entry> <exec>
sldm remove <entry>
sldm list [entry]
sldm show <entry>
sldm [options] [entry] - Enter the menu screen

options:
  -r    force run xorg if an entry name is the same as the commands above
```  
- ### **Configuration**
Although there isn't much to configure as it is pretty simple tool, there are few options in config.h inside cloned repo.
Configuration is provided with C language macroses:
```
PROMPT_TIMEOUT [10]
DEFAULT_ENTRY [1]
BASE_XCONFIG ["~/.xinitrc"]
