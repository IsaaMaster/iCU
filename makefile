all:
	gcc client.c -o client -lcurl
	gcc server.c utils.c -o server
	gcc driver.c -o driver
