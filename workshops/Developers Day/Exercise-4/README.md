# Tutorial: Building an RPM Package for a C HTTP Application

This tutorial will guide you through the process of creating a Red Hat Package Manager (RPM) package for your multi-threaded C HTTP application. RPMs are a standard way to distribute, install, upgrade, and remove software on RHEL, Fedora, CentOS, and other RPM-based Linux distributions.

## 1. Introduction to RPM Packaging

An RPM package (.rpm file) is an archive that contains the necessary files to install a piece of software, along with metadata about the software, its dependencies, and instructions on how to install and uninstall it. Building an RPM involves:

Source Code: Your application's source code and build system (e.g., Autotools).

Spec File: A crucial file (.spec) that describes the package metadata, build instructions, installation locations, and file lists.

RPM Build Environment: A specific directory structure where the source code, build artifacts, and package files are placed during the RPM build process.

## 2. Prerequisites

Before you start, ensure you have:

Your multi-threaded C HTTP application with its Autotools build system (configure.ac, Makefile.am, http_service.c) set up and working. You should be able to compile and install it using make && sudo make install.

- A RHEL-based system with the necessary RPM development tools installed.

- Root privileges (sudo) for installing packages.

- Install RPM Build Tools

If you don't have the RPM building tools, install them:
```bash
sudo dnf install rpm-build redhat-rpm-config -y
```
Set up RPM Build Environment

RPM build typically uses a specific directory structure under your home directory. Create it if it doesn't exist:
```bash
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
```    
- BUILD: Where the software is actually built (extracted and compiled).

- RPMS: Where the final .rpm binary packages are stored (e.g., x86_64, noarch).

- SOURCES: Where compressed source archives (tarballs) and patches are placed.

- SPECS: Where the .spec files are located.

- SRPMS: Where the source RPM packages are stored.

## 3. Preparing Your Source Code

Your application's source code, along with its Autotools files, needs to be packaged into a compressed archive (tarball). This tarball will be placed in the ~/rpmbuild/SOURCES directory.

Navigate to your project directory:
```bash
cd /path/to/your/project/
```
(Replace /path/to/your/project/ with your actual project location)

Generate a distribution tarball: Use make dist (from your Autotools setup) to create a standard source tarball.
```bash
make dist
```
This will typically create a file like http_service-1.0.tar.gz in your project directory.
Move the tarball to SOURCES:
```bash
mv http_service-1.0.tar.gz ~/rpmbuild/SOURCES/
```

## 4. Creating the .spec File

The .spec file is the heart of your RPM package. It contains metadata and instructions for building and installing your application.

Create the .spec file:
```bash
    nano ~/rpmbuild/SPECS/http_service.spec
```
    
Add the content to http_service.spec:

    Name:           http_service
    Version:        1.0
    Release:        1%{?dist}
    Summary:        A multi-threaded C HTTP service
    
    License:        MIT # Or GPLv3+, BSD, etc. Based on your project's license
    Source0:        http_service-%{version}.tar.gz 
    
    BuildRequires:  gcc
    BuildRequires:  make
    BuildRequires:  autoconf
    BuildRequires:  automake
    BuildRequires:  libtool
    # If your service uses other libraries beyond standard ones, add their -devel packages here
    # For pthreads, it's typically part of glibc-devel, which gcc depends on.
    
    Requires: systemd 
    
    %description
    This is a simple multi-threaded HTTP service written in C.
    It accepts JSON input with a 'name' and 'sentence' and
    responds with the hostname and the original sentence.
    
    %prep
    # This section extracts the source tarball and prepares the build directory.
    # %setup -q is a macro that extracts Source0 and changes into the extracted directory.
    %setup -q
    
    %build
    # This section contains the commands to build your software.
    # For Autotools, it's typically:
    %configure
    %make_build
    
    %install
    rm -rf %{buildroot}
    %make_install
    
    install -D -m 0644 %{_builddir}/http_service-%{version}/http-service.service \
        %{buildroot}%{_unitdir}/http-service.service
    
    %files
    %{_bindir}/http_service
    # Service file
    %{_unitdir}/http-service.service

    %changelog
    * Mon Jun 10 2025 Your Name <your_email@example.com> - 1.0-1
    - Initial RPM package creation

Key sections and macros in the .spec file:

- Header: Defines package name, version, release, summary, license, URL, and source tarball.

- Release: 1%{?dist}: %{?dist} automatically adds distribution-specific tags (e.g., .el9 for RHEL 9).

- Source0: Points to your source tarball.

- BuildRequires: List of packages (development headers and libraries) that must be present on the build system to compile your software.

- Requires: List of packages that must be present on the target system for your installed software to run. Since we're making a Systemd service, systemd is a common runtime dependency.

- %description: A longer description of your package.

- %prep: Prepares the source code. %setup -q extracts Source0 into the BUILD directory.

- %build: Contains commands to compile the software. For Autotools projects, %configure and %make_build macros simplify this.

- %install: Installs the compiled software into a temporary staging directory (%{buildroot}). %make_install runs make install relative to %buildroot.

- Systemd Service File Installation: Notice the install -D command. This assumes you will have your http-service.service file (from the previous tutorial) placed within your source distribution (e.g., in the same directory as http_service.c). The %{_unitdir} macro expands to /usr/lib/systemd/system/ on most Systemd systems, which is the correct place for system services.

- %files: Lists all files that will be included in the final RPM package. Any file installed in %install must be listed here.

- %{_bindir}: RPM macro for /usr/bin or /usr/local/bin, depending on the build configuration.

- %changelog: Documents changes made to the package.

Place http-service.service in your project

For the install -D command in %install to work, you need to ensure the http-service.service file is part of your source distribution. The simplest way for this tutorial is to copy it into your project's main directory (where http_service.c and Makefile.am are).
```bash
cp /etc/systemd/system/http-service.service /path/to/your/project/
```

Then, regenerate your source tarball (make dist and move it to ~/rpmbuild/SOURCES/) so the service file is included.

## 5. Building the RPM Package

With the source tarball in SOURCES and the .spec file in SPECS, you can now build the RPM.

Navigate to your home directory or any convenient location:
```bash
cd ~
```

Execute the rpmbuild command:
```bash
rpmbuild -ba ~/rpmbuild/SPECS/http-service.spec
```
- -ba: Stands for "build all" (binary and source RPM).
- ~/rpmbuild/SPECS/http-service.spec: The path to your spec file.

rpmbuild will:

- Extract the source tarball into ~/rpmbuild/BUILD/.

- Execute the commands in the %prep, %build, and %install sections.

- Collect the files listed in %files from %buildroot.

- Create the .rpm package in ~/rpmbuild/RPMS/<architecture>/ (e.g., ~/rpmbuild/RPMS/x86_64/).

- Create a source RPM (.src.rpm) in ~/rpmbuild/SRPMS/.

Verify Package Creation:
```bash
    ls -l ~/rpmbuild/RPMS/$(uname -i)/
```

You should see your http-service-1.0-1.<arch>.rpm file. For example:
    http-service-1.0-1.el9.x86_64.rpm

## 6. Installing and Testing the RPM Package

Once you have built the RPM, you can install it on your RHEL system.

Install the RPM:
```bash
sudo yum install ~/rpmbuild/RPMS/$(uname -i)/http-service-1.0-1.$(uname -i).rpm
```
# OR for newer RHEL/Fedora versions:
```bash
sudo dnf install ~/rpmbuild/RPMS/$(uname -i)/http-service-1.0-1.$(uname -i).rpm
```
    
This command will install your http_service executable to /usr/local/bin/ (or /usr/bin/ depending on your Autotools configure prefix) and the http-service.service file to /usr/lib/systemd/system/.

Enable and Start the Service:
```bash
    sudo systemctl daemon-reload
    sudo systemctl enable --now http-service
```

Check Status and Test:
```bash
    sudo systemctl status http-service
```
And we can reuse our CURL command to test the server :
```bash
    curl -X POST -H "Content-Type: application/json" \
         -d '{"name": "RPM Test", "sentence": "Testing RPM-installed service!"}' \
         http://localhost:8080
```
We can Even look at to journal to see if we find anything about our http-service service
```bash
sudo journalctl -u http-service.service -f
```
**Not Mandatory**
Uninstall the RPM:
```bash
sudo yum remove http-service
```    
# OR
```bash
sudo dnf remove http-service
```

This will remove the installed files and disable the Systemd service.

## 7. Advanced RPM Concepts (Briefly)

- Macros: RPM uses many built-in macros (e.g., %{_bindir}, %{_unitdir}) to define standard system paths.

- Patching: You can apply patches to your source code in the %prep section if needed.

- Signing: RPMs can be GPG-signed for authenticity verification.

- Dependencies: More complex dependency management can be done in BuildRequires and Requires.

- Post-install/Pre-uninstall scripts: %post and %preun sections allow you to run scripts after installation or before uninstallation, respectively (e.g., systemctl daemon-reload and systemctl enable could theoretically be put here, but it's often better to leave service management to the user).

Building RPMs can be complex for large projects, but for a simple application like this, the .spec file is relatively straightforward. This process ensures your application is properly packaged and integrated into the RHEL system.
