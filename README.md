# 11_webserv

## Project overview

The objective of this project is to create your own HTTP server. It should be inspired by Nginx and run on a web browser.

## How to use this repository?

### Recommended Operating System (OS)

I recommand to use a Linux distribution such as:

- Latest stable version (LTS) of Ubuntu
- Latest stable version (LTS) of Debian

### Steps to follow

1. Go to the directory where you want to clone the directory: `cd path/of/repository/`
2. Clone this repository: `git clone git@github.com:romainkassel/11_webserv.git`
3. Enter into cloned repository: `cd 11_webserv`
4. Launch the project's build: `make`
5. Start the server: `./webserv`
6. Here you go! If everything went smoothly, the server should be listening and waiting for requests!

## Cool! How can I query the server now?

By default, the server is listening on `localhost` address (`127.0.0.1`) and binded on the port `8080`

You can query the server in two different ways.

### Using CURL

You can use your terminal by:
1. Opening a new shell/window instance (the server should be running and listening on the other one)
2. Running the `curl` command in the new shell: `curl localhost:8080`.

The server will receive the request and return the following response: 

<img width="1470" height="112" alt="webserv-curl-response-index-html" src="https://github.com/user-attachments/assets/55ac4a50-5480-443d-b550-bd624cdb19aa" />

It relates to the HTML document located at `www/index.html`.

### Using your browser

If you are a more visual person, you can:
1. Open your favorite browser (Firefox recommended)
2. Enter the following URL in the address bar: `http://localhost:8080`

<img width="1470" height="324" alt="webserv-browser-response-index-html" src="https://github.com/user-attachments/assets/b1a22608-cf3c-4656-bd63-22fbb7dcc148" />

> [!NOTE]
> Each time you will query the server, it will log client connections on the console standard output:

<img width="282" height="370" alt="webserv-terminal-client-connections" src="https://github.com/user-attachments/assets/06c363bb-fbef-4665-af29-d044c8da7b70" />

## Can I customise my server?

Like Nginx, yes you can!

By default, with no argument, the server use the following configuration file: `conf/default.conf`.

You can create your own or select one of the configuration files that we have created for you and for our tests in one of the subfolders located within `conf/tests`.

As Nginx does, you can:

- select a custom port for listening within the authorised range, in which case an error will be thrown
- create multiple server directives: `server {}`
- within each server directive, you can create multiple location directives with different modifiers (`/` or `=` for example)
- play with locations by creating folders and HTML files and see if the server will serve the content or throw a 404 error

> [!TIP]
> If you want explanations and inspiration for test cases, you can check this article from DigitalOcean: [Nginx location directive examples](https://www.digitalocean.com/community/tutorials/nginx-location-directive)
> You can also use our own location folders and files located within the `default/` folder

## Which System Call has been used for the IO Multiplexing part?

When building a server, you have 3 options for polling 



## I tested the site and I'm happy. Now I'd like to clean it up. What do I do?

1. In your terminal, stop the server by clicking on `CTRL + C`
2. Go outside of the repository: `cd ..`
3. Remove the repository: `rm -rf 11_webserv`
