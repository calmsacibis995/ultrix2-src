if [ ! -r version ]; then echo 0 > version; fi
touch version
awk '	{	version = $1 + 1; }\
END	{	printf "char version[] = \"Ultrix V2.0-1 System #%d: ", version > "vers.c";\
		printf "%d\n", version > "version"; }' < version
echo `date`'\n";' >> vers.c
