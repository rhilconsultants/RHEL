# Building and Testing a Multi-threaded C HTTP Service on RHEL

This tutorial will guide you through the process of setting up, compiling, and testing the multi-threaded C HTTP JSON service application on a Red Hat Enterprise Linux (RHEL) environment.
## 1. Introduction

The application is a simple HTTP service written in C that listens for incoming POST requests, expects a JSON payload containing a "name" and a "sentence", and responds with a JSON structure that includes the server's hostname and the received sentence. The multi-threaded design allows it to handle multiple client connections concurrently.

## 2. Prerequisites

Before you begin, ensure you have the following tools installed on your RHEL system:

GCC (GNU Compiler Collection): The C compiler.

To install: 
```bash
sudo yum install gcc or sudo dnf install gcc
```

curl: A command-line tool for making HTTP requests (useful for testing).

To install: 
```bash
sudo yum install curl or sudo dnf install curl
```  

## 3. Writing (Saving) the Application

The C source code for the multi-threaded HTTP JSON service is provided in the http-json-service immersive artifact.

Access the Code: Copy the entire content from the http-json-service immersive artifact which is located [here](https://raw.githubusercontent.com/rhilconsultants/RHEL/refs/heads/main/workshops/Developers%20Day/Code/http_service.c.)

Create a File: Open a text editor on your RHEL system (e.g., nano, vim, gedit) and paste the copied code into a new file.

Save the File: Save the file as http_service.c in a directory of your choice (e.g., your home directory or a new project folder).

Example using nano:
```bash
nano http_service.c
```
OR you can use VI
```bash
vi http_service.c
```

(Paste the code, then Ctrl+X, Y, Enter to save and exit)

## 4. Compiling the Application

To compile the application, you'll use GCC and specifically link against the POSIX threads library (pthread), as the application is multi-threaded.

Open Terminal: Navigate to the directory where you saved http_service.c.
```bash
cd /path/to/your/project/
```

(Replace /path/to/your/project/ with your actual directory)

Compile Command: Execute the following command:
```bash
gcc http_service.c -o http_service -lpthread
```
   gcc: Invokes the GNU C compiler.

   http_service.c: Your source code file.

   -o http_service: Specifies that the compiled executable should be named http_service.

   -lpthread: This crucial flag links your program with the pthread library, which provides the multi-threading functionalities (pthread_create, pthread_detach, etc.). Without this, the compiler would report undefined references to these functions.

Verify Compilation: If the compilation is successful, you won't see any output (or only warnings, which you should ideally address in a real-world scenario). A new executable file named http_service will be created in your current directory. You can check its existence using:

```bash
 ls -l http_service
```

If you encounter errors like undefined reference to 'pthread_create', it means you forgot the -lpthread flag or it's not correctly configured.

## 5. Running the Application

Once compiled, you can run your HTTP service.

Execute: From your terminal, in the same directory as the executable, run:

```bash
./http_service
```

Expected Output: You should see output similar to this, indicating the server has started and is listening for connections:
Server listening on port 8080. Hostname: your-rhel-hostname
Ready to accept connections...
(Your actual hostname will be displayed.)
The server will now be running and waiting for incoming HTTP requests on port 8080.

## 6. Testing the Application

You can test the service using curl from another terminal or even the same one (if you send it to the background, though it's easier with a new terminal).

Open a New Terminal: Keep the server running in its original terminal.

Send a POST Request: Use curl to send a POST request with a JSON body to your server.

```bash
    curl -X POST -H "Content-Type: application/json" \
         -d '{"name": "RHEL Tester", "sentence": "This is a test sentence from RHEL!"}' \
         http://localhost:8080
```

   -X POST: Specifies the HTTP method as POST.

   -H "Content-Type: application/json": Sets the Content-Type header, informing the server that the request body is JSON.

   -d '...': Provides the request body. The JSON string contains name and sentence fields.

   http://localhost:8080: The address and port of your running HTTP service.

Observe Client Output: On the client terminal (where you ran curl), you should receive a JSON response:

   {"hostname": "your-rhel-hostname", "sentence": "This is a test sentence from RHEL!"}

(Again, your-rhel-hostname will be your machine's actual hostname.)

Observe Server Output: In the terminal where your http_service is running, you'll see logs indicating the connection, the received request, the extracted sentence, and the sent response, with messages from the individual threads handling the connections:

    Main thread: Connection accepted from 127.0.0.1:xxxxx. Creating new thread...
    Thread handling connection from 127.0.0.1:xxxxx (socket X)
    Received request on socket X:
    POST / HTTP/1.1
    Host: localhost:8080
    User-Agent: curl/7.76.1
    Accept: */*
    Content-Type: application/json
    Content-Length: 64

    {"name": "RHEL Tester", "sentence": "This is a test sentence from RHEL!"}

    Extracted sentence: 'This is a test sentence from RHEL!' from socket X
    Response sent on socket X:
    HTTP/1.1 200 OK
    Content-Type: application/json
    Content-Length: 72
    Connection: close

    {"hostname": "your-rhel-hostname", "sentence": "This is a test sentence from RHEL!"}
    Connection on socket X closed. Thread exiting.

Test Concurrency: To see the multi-threading in action, open several new terminal windows and run the curl command simultaneously or in rapid succession. You'll observe the server handling these requests in parallel, with multiple "Thread handling connection..." messages appearing without blocking each other.

## 7. Troubleshooting Common Issues

   "Address already in use" error: This means another process is already listening on port 8080. Either stop that process or change the PORT define in http_service.c to a different value (e.g., 8081), then recompile and rerun.

   "Connection refused" error (from curl): The server is not running, or your firewall is blocking the connection.

   Ensure ./http_service is running.

   Check firewall: 
```bash
sudo firewall-cmd --list-all 
```
or 
```bash
sudo systemctl status firewalld
``` 
You might need to open port 8080: 
```bash
sudo firewall-cmd --zone=public --add-port=8080/tcp --permanent
```
then
```bash
sudo firewall-cmd --reload
```

   undefined reference to 'pthread_create': As mentioned, you forgot to include -lpthread during compilation.

   JSON parsing issues: If the server doesn't extract the sentence correctly, double-check your curl command's JSON format. The simple parser in the C code is very sensitive to exact formatting.

   Server crashes: This can happen with more complex inputs or larger data if buffers are too small. For robust applications, dynamic memory allocation and more sophisticated error handling are needed.

## 8. Further Improvements

   Robust JSON Parsing: Integrate a proper JSON parsing library like cJSON or Jansson.

   HTTP Protocol Handling: Implement full HTTP request parsing (headers, different methods, query parameters).

   Error Handling and Logging: More detailed error messages and logging to files.

   Graceful Shutdown: Implement a way to shut down the server cleanly (e.g., signal handling).

   Scalability: For very high concurrency, consider an event-driven I/O model (e.g., using epoll on Linux) instead of a thread-per-connection model, which can consume significant resources with many concurrent connections.

   HTTPS Support: For secure communication, integrate SSL/TLS libraries (like OpenSSL).

This tutorial should give you a solid foundation for deploying and testing your C HTTP service on RHEL!
