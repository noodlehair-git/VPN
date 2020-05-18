all: compile_client compile_server

compile_server: 
		gcc supergopher.c -o supergopher

compile_client:   
		gcc minigopher.c -o minigopher



