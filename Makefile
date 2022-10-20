CXX = gcc

all: ParentProcess LikeServer PrimaryLikesServer

ParentProcess: ParentProcess.c 
	$(CXX) ParentProcess.c -o ParentProcess

LikeServer: LikeServer.c
	$(CXX) LikeServer.c -o LikeServer0

PrimaryLikesServer: PrimaryLikesServer.c
	$(CXX) PrimaryLikesServer.c -o PrimaryLikesServer

run:
	./ParentProcess

clean:
	rm /tmp/LikeServer* /tmp/ParentProcessStatus /tmp/PrimaryLikesLog ParentProcess LikeServer0 PrimaryLikesServer