[ -d bcpp ] && rm -rf bcpp
mkdir bcpp
cp *.c *.h bcpp
cd bcpp || exit
for i in *; do bcpp $i > ../$i; done
cd -
