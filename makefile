target: makeFileSystem fileSystemOper

makeFileSystem: makeFileSystem.cpp
		g++ makeFileSystem.cpp -o makeFileSystem -Wall

fileSystemOper: fileSystemOper.cpp
		g++ fileSystemOper.cpp -o fileSystemOper -Wall

clean:
		rm makeFileSystem fileSystemOper