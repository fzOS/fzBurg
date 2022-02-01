all:main
main:
	bash -c "cd .. && . edksetup.sh && build"
clean:
	bash -c "rm -rf ../Build/FzBurgPkg/"
