all:
	gcc client.c utils.c -o client -lcurl
	gcc server.c utils.c -o server
	gcc driver.c -o driver
