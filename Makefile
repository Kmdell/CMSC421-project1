CXX = gcc

ParentProcess: ParentProcess.c
	$(CXX) ParentProcess.c -o ParentProcess

run:
	./ParentProcess

clean:
	rm /tmp/LikeServer* /tmp/ParentProcessStatus /tmp/PrimaryLikesLog ParentProcess