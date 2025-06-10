# Tutorial: Building a Systemd Service for a C HTTP Application

This tutorial will guide you through creating a Systemd service unit file for your multi-threaded C HTTP application, allowing it to run as a background service, start automatically on boot, and be managed efficiently on RHEL (and other Linux distributions using Systemd).

## 1. Introduction to Systemd

Systemd is a system and service manager for Linux operating systems. It is designed to be backwards compatible with SysV and LSB init scripts, and provides a powerful and flexible framework for controlling services on your system. Key features of Systemd include:

   Parallelization: Services can start in parallel, significantly speeding up boot times.

   On-demand starting: Services can be started only when they are needed.

   Dependency-based control: Services are started in the correct order based on their dependencies.

   Logging: Integrates with journald for centralized logging.

   Process management: Keeps track of processes using Linux control groups (cgroups).

For our C HTTP application, creating a Systemd service means it will automatically start when the system boots, restart if it crashes, and you can easily manage its state (start, stop, restart, status) using the systemctl command.

## 2. Prerequisites

Before proceeding, ensure you have:

- Your multi-threaded C HTTP application compiled and working. You should have the http_service executable ready. It's usually good practice to have it installed to a standard location (e.g., /usr/local/bin/) using make install after compiling with Autotools.

- Root privileges (sudo) to manage Systemd services.

## 3. Creating the Systemd Service Unit File

A Systemd service is defined by a unit file, typically with a .service extension. These files are usually located in /etc/systemd/system/.

    Open a text editor with sudo:

    sudo nano /etc/systemd/system/http-service.service

    (You can choose any name for the service, but http-service.service is descriptive.)

    Add the service unit file content: Paste the following content into the file. Read the comments carefully for explanations of each directive.

    [Unit]
    # Description of your service
    Description=Multi-threaded C HTTP Service
    # Define dependencies. 'network-online.target' ensures network is up before starting.
    After=network-online.target

    [Service]
    # Type of service: 'simple' means the process itself is the main service.
    Type=simple
    # User and group under which the service will run.
    # It's highly recommended to run services as a non-root user for security.
    # Replace 'youruser' with an actual unprivileged user on your system.
    # If 'youruser' doesn't exist, you might need to create it: sudo useradd -r youruser
    User=youruser 
    Group=youruser

    # The working directory for the service.
    # If your service needs to access files relative to its executable, set this.
    # Otherwise, it can be left out, but good practice.
    # Assuming http_service is installed to /usr/local/bin/
    WorkingDirectory=/usr/local/bin/

    # The command to execute when the service starts.
    # Use the full path to your executable.
    # If you used 'sudo make install', it's likely in /usr/local/bin/.
    ExecStart=/usr/local/bin/http_service

    # Restart policy:
    # 'on-failure' means restart if the process exits with a non-zero exit code.
    # Other options: 'always', 'on-success', 'on-abnormal', 'on-watchdog', 'on-abort'.
    Restart=on-failure
    # How long to wait before restarting (e.g., 5 seconds)
    RestartSec=5s

    # Standard output and error configuration.
    # 'journal' sends output to the Systemd journal (viewable with journalctl).
    StandardOutput=journal
    StandardError=journal

    # (Optional) Set resource limits if needed.
    # LimitNOFILE=1024 # Max number of open file descriptors

    [Install]
    # Defines the target that this service should be enabled for.
    # 'multi-user.target' means it will start when the system enters multi-user mode (normal boot).
    WantedBy=multi-user.target

    Important:

        User/Group: Change youruser to an actual unprivileged user and group on your system. Running services as root is a security risk unless absolutely necessary.

        ExecStart Path: Ensure the path /usr/local/bin/http_service correctly points to your compiled executable. If you installed it elsewhere (e.g., /usr/bin/), adjust the path accordingly. If you did not make install, then it would be ./http_service in your project folder, which would require a different WorkingDirectory or a full path from your home directory.

4. Managing the Systemd Service

Once the .service file is created, you can use the systemctl command to manage your service.

    Reload Systemd Daemon: After creating or modifying any unit file, you must tell Systemd to reload its configuration.

    sudo systemctl daemon-reload

    Enable the Service: Enabling the service ensures it starts automatically on system boot.

    sudo systemctl enable http-service

    This creates a symbolic link from /etc/systemd/system/multi-user.target.wants/http-service.service to your service file.

    Start the Service:

    sudo systemctl start http-service

    The service should now be running in the background.

    Check Service Status: To verify if the service is running and to see recent logs:

    sudo systemctl status http-service

    You should see output indicating it's "active (running)" and recent log entries.

    Test the Application: Now that the service is running, you can test it using curl as you did before:

    curl -X POST -H "Content-Type: application/json" \
         -d '{"name": "Systemd Test", "sentence": "Testing service via Systemd!"}' \
         http://localhost:8080

    You should get a JSON response back. The output of your C application (e.g., "Received request...") will now be sent to the Systemd journal.

    View Service Logs (Journal): To see the logs generated by your application (standard output/error) via Systemd's journal:

    sudo journalctl -u http-service.service -f

        -u http-service.service: Filters logs for your specific service.

        -f: "Follow" mode, which displays new log entries as they arrive (like tail -f). Press Ctrl+C to exit follow mode.

    Stop the Service:

    sudo systemctl stop http-service

    You can check its status again to confirm it's "inactive (dead)".

    Restart the Service:

    sudo systemctl restart http-service

    This is a convenient way to stop and then start the service. Useful after making changes to your application's executable.

    Disable the Service: If you no longer want the service to start on boot:

    sudo systemctl disable http-service

    This removes the symbolic link created by enable. It does not stop a currently running service.

5. Best Practices and Considerations

    Security: Always run services as an unprivileged user (User= and Group= directives). Create a dedicated user for your service if one doesn't exist.

    Logging: Using StandardOutput=journal and StandardError=journal is the recommended way to handle logging with Systemd. journalctl provides powerful filtering and querying capabilities.

    ExecStart Path: Always use absolute paths for executables in ExecStart.

    Working Directory: Define WorkingDirectory if your application needs to access files relative to its location.

    Restart Policy: Choose a Restart policy that suits your application's needs (e.g., on-failure is common for robust services).

    Environment Variables: You can set environment variables for your service using Environment=KEY=VALUE or EnvironmentFile=/path/to/env_file.

    Dependencies: Use After= and Requires= (if a hard dependency) to ensure your service starts at the correct time in the boot sequence.

By following this tutorial, you've successfully containerized your C HTTP application within a Systemd service, making it a robust and manageable background process on your RHEL system.
