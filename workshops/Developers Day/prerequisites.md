# Lab Prerequisites

This page outlines the necessary tools and configurations required on your Red Hat Enterprise Linux (RHEL) system to successfully complete the lab. Please ensure all prerequisites are met before proceeding.

## 1. Operating System

 Red Hat Enterprise Linux (RHEL): This lab is designed and tested on RHEL 9. While concepts might apply to other Linux distributions, specific commands for package management (dnf) are RHEL-centric.

## 2. System Access

- WEB/SSH Access: You need either SSH (ask the Instractor for login credentials) or the default WEB to access your RHEL machine.

- sudo Privileges: You must have sudo privileges to install packages and manage system services (like Systemd).

## 3. Terminal Enhancements

### 3.1. tmux (Terminal Multiplexer)

tmux is a powerful terminal multiplexer that allows you to create multiple independent terminal sessions within a single window. This is incredibly useful for managing multiple processes (like running the server and sending requests) and for persistent sessions that survive disconnections.

Installation:
```bash
sudo dnf install -y tmux
```

Verification:
To ensure tmux is installed and functioning, you can check its version.
```bash
tmux -V
```
    
You should see output similar to:
```
tmux 3.2a
```

For those of you who don't know tmux in a very powerful tool which allows us to run terminal manipulation in various forms. In our case we would want to slip the screen to 3 parts (vertical middle and 2 horizontal on the top side) to enable us better monitoring on all the process.

Here is how we do it:

First modify the tmux configuration file as follows:

#### For Linux Users

```bash
$ cat > ~/.tmux.conf << EOF
unbind C-b
set -g prefix C-a
bind -n C-Left select-pane -L
bind -n C-Right select-pane -R
bind -n C-Up select-pane -U
bind -n C-Down select-pane -D
EOF
```

#### For Windows & MAC Users

```bash
$ cat > ~/.tmux.conf << EOF
unbind C-b
set -g prefix C-a
bind -n M-Left select-pane -L
bind -n M-Right select-pane -R
bind -n M-Up select-pane -U
bind -n M-Down select-pane -D
EOF
```

Now start a tmux session:

```bash
tmux new-session -s rhel
```

#### Spliting the screen (NOT Mandatory)

Now we will split the screen by clicking on CTRL+a then '"'.  
Next we will Navigate to the top bar by CTRL+UP (the ARROW UP) and create another slip horizontally by running CTRL+a then "%"  
To navigate between them you can run CTRL+ARROW and the arrows.  


### 4.2. jq (JSON Processor)

jq is a lightweight and flexible command-line JSON processor. It's invaluable for parsing, filtering, and manipulating JSON data directly in your terminal, which will be useful when working with the JSON responses from your HTTP service.

Installation:
```bash
    sudo dnf install -y jq
```
    
Verification:
To confirm jq is installed and ready, check its version.
```bash
jq --version
```

You should see output similar to:
```
jq-1.6
```
    
Quick jq Usage Example:

Parse a JSON string:

        echo '{"name": "Alice", "age": 30}' | jq .name
        # Output: "Alice"

Pretty-print JSON:

        echo '{"name":"Bob","city":"New York"}' | jq .
        # Output:
        # {
        #   "name": "Bob",
        #   "city": "New York"
        # }

With these prerequisites in place, your RHEL environment will be ready for the lab exercises.
