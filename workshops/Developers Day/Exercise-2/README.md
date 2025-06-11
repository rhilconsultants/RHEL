# Autotools Tutorial for a Multi-threaded C HTTP Service

This tutorial provides a focused guide on how to set up and use Autotools (Autoconf and Automake) to manage the build process for your multi-threaded C HTTP service application. Autotools helps create a portable build system that can be compiled on various Unix-like operating systems.

## 1. Introduction to Autotools

Autotools is a suite of programming tools designed to assist in creating portable software for Unix-like operating systems. It helps developers write code that can be compiled on a wide range of systems without manual configuration adjustments. The key components are:

   Autoconf: Generates a configure script from configure.ac. This script detects system-specific features (like the presence of libraries or header files) and creates a Makefile suitable for that system.

   Automake: Generates Makefile.in files from Makefile.am. These Makefile.in files are then used by the configure script to create the final Makefile.

   Libtool: (Often used with Autotools) Helps manage the creation and installation of shared libraries across different platforms. While not strictly necessary for this simple executable, it's a common companion.

## 2. Prerequisites

Before you start, ensure you have Autotools installed on your RHEL system:

Autotools (Autoconf, Automake, Libtool):
To install on RHEL (using yum or dnf):
```bash        
sudo dnf install autoconf automake libtool -y 
```

Your C Source File: Make sure you have your application's source code saved as http_service.c in your project directory.

## 3. Setting up Autotools Files
You'll need to create two main files in your project's root directory: configure.ac and Makefile.am.

### 3.1. Create configure.ac
The configure.ac file is where you define system checks and output files for Autoconf.
Create the file: In your project directory, create a file named configure.ac using your preferred text editor (e.g., nano):

```bash
nano configure.ac
```

Add the content: Paste the following into configure.ac:

    # Process this file with autoconf to produce a configure script.

    # Initialize Autoconf with your package details:
    # PACKAGE_NAME, VERSION, BUG_REPORT_EMAIL
    AC_INIT([http_service], [1.0], [your_email@example.com])

    # Check for a C compiler (e.g., gcc)
    AC_PROG_CC

    # Initialize Automake with options:
    # -Wall: Enable all warnings for generated Makefiles
    # -Werror: Treat warnings as errors
    # foreign: Relax GNU standards (e.g., no need for NEWS, AUTHORS files)
    AM_INIT_AUTOMAKE([-Wall -Werror foreign])

    # Check for the 'pthread' library and the 'pthread_create' function.
    # If not found, display an error message and stop configuration.
    AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_ERROR([pthread library not found])])

    # Generate a config.h header file, which can contain system-specific definitions.
    AC_CONFIG_HEADERS([config.h])

    # Specify that a Makefile should be generated from Makefile.in.
    AC_CONFIG_FILES([Makefile])

    # Perform the final output actions, generating the specified files.
    AC_OUTPUT


### 3.2. Create Makefile.am

The Makefile.am file is the input for Automake. It describes the structure of your project, including which executables to build and which source files and libraries they require.
Create the file: In the same project directory, create a file named Makefile.am:
```bash
nano Makefile.am
```
Add the content: Paste the following into Makefile.am:

    # Process this file with automake to produce Makefile.in

    # Declare the programs to be built and installed.
    # 'bin_PROGRAMS' specifies that 'http_service' is an executable
    # that should be installed into the 'bindir' (e.g., /usr/local/bin).
    bin_PROGRAMS = http_service

    # List the source files required to build the 'http_service' executable.
    http_service_SOURCES = http_service.c

    # Specify additional libraries that 'http_service' needs to link against.
    # This is where we tell the linker to include the 'pthread' library.
    http_service_LDADD = -lpthread


## 4. Generating the Build System

Once configure.ac and Makefile.am are in place, you use Autotools commands to generate the configure script and Makefile.in files, which are then used to create the final Makefile.

Run autoreconf: This command is the primary entry point for Autotools. It runs aclocal, autoconf, and automake in the correct order to generate the build system.
```bash
autoreconf --install --force
```

   --install: Installs any necessary helper files (like install-sh, missing, depcomp) that are required by the generated Makefiles.

   --force: Forces reprocessing, even if the input files haven't changed. This is often useful during initial setup or when debugging Autotools configurations.

   You will see a series of commands executed (e.g., aclocal, autoconf, autoheader, automake). After this step, you should have a configure script and Makefile.in file (among others) in your directory.

## 5. Compiling the Application using the Generated Build System
Now that the build system is generated, the compilation process follows a standard three-step approach.

Remove the binary from the last exercise :
```bash
rm -f http_service
```

Run ./configure: This script performs system-specific checks and generates the final Makefile tailored to your environment.

```bash
./configure
```

You will see output detailing the checks being performed (e.g., checking for gcc, checking for pthread_create). If everything is successful, it will finish by creating the Makefile.
   
Run make: This command reads the generated Makefile and compiles your source code.

```bash
make
```

This will compile http_service.c and link it with the pthread library to produce the http_service executable.

Verify Compilation:
```bash
ls -l http_service
```
You should see the http_service executable file created in your project directory.

## 6. Installing (Optional)

Autotools also provides standard targets for installing and uninstalling your application.

Install: To install the executable to the standard system binary directory (usually /usr/local/bin/):
```bash
sudo make install
```
After installation, you can run http_service from any directory without needing ./http_service.
Make sure the file is at /usr/local/bin
```bash
ls -l /usr/local/bin/http_service
```

## 7. Testing the Application

Now run the same test from the previuse exercise :

Execute: From your terminal, in the same directory as the executable, run:

```bash
/usr/local/bin/http_service
```

Expected Output: You should see output similar to this, indicating the server has started and is listening for connections:
Server listening on port 8080. Hostname: your-rhel-hostname
Ready to accept connections...
(Your actual hostname will be displayed.)
The server will now be running and waiting for incoming HTTP requests on port 8080.

You can test the service using curl from another terminal/tmux session or even the same one (if you send it to the background, though it's easier with a new terminal).

Open a New Terminal: Keep the server running in its original terminal.

Send a POST Request: Use curl to send a POST request with a JSON body to your server.

```bash
    curl -X POST -H "Content-Type: application/json" \
         -d '{"name": "RHEL Tester", "sentence": "This is a test sentence from RHEL!"}' \
         http://localhost:8080
```

## 8. Unistallation the Application (only if you did section 6)

Uninstall: To remove the installed files:

```bash
sudo make uninstall
```

## 9. Troubleshooting Autotools Issues

   autoreconf: command not found: This means Autotools is not installed. Revisit Section 2.

   configure.ac: ... AC_INIT: not found (or similar Autoconf macro errors): This indicates that autoconf is not properly installed or accessible in your PATH.

   Makefile.am: ... bin_PROGRAMS: not found (or similar Automake macro errors): This indicates that automake is not properly installed or accessible.

   configure: error: pthread library not found: This specific error comes from the AC_CHECK_LIB line in your configure.ac. It means Autoconf could not find the pthread library on your system during the ./configure step. Ensure pthread development packages are installed (usually part of glibc-devel on RHEL, which gcc might pull in, but worth checking).

   undefined reference to 'pthread_create' (during make after successful ./configure): This is less common with a correct AC_CHECK_LIB and _LDADD setup but could indicate an issue with your Makefile.am or a very unusual system setup. Double-check http_service_LDADD = -lpthread in Makefile.am.

   No such file or directory for http_service.c: Ensure http_service.c is in the same directory as Makefile.am.

By following these steps, you can effectively use Autotools to manage the build process for your C HTTP service, making it more robust and portable.
