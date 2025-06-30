import sys
import os

print("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>WebServ</title></head><body><h1>Webserv</h1><h2>Files :</h2><div><ul>")

# Recupere la liste des fichiers uploades sur le serveur
for dir in os.listdir(sys.argv[1]):
	print("<li>",dir,"</li>")

print("</ul></div></body></html>\0")

sys.exit(200)
