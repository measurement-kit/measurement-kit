#!/bin/bash

#Versione di sdk e minima versione di IOS per cui compilare
SDKVERSION="8.1"
MINIOSVERSION="6.1"

#Vettore contenente architetture
ARCHS="i386 x86_64 armv7 arm64"

DEVELOPER=`xcode-select -print-path`
REPOROOT=$(pwd)

#Creo cartelle di output per libreria *header e lib statica*
OUTPUTDIR="${REPOROOT}/dependencies"
mkdir -p ${OUTPUTDIR}/include
mkdir -p ${OUTPUTDIR}/lib

BUILDDIR="${REPOROOT}/build"

INTERDIR="${BUILDDIR}/built"
mkdir -p $INTERDIR

#Inserirsco in un articolo due librerie dinamiche dell'sdk
lipo -create ${DEVELOPER}/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator${SDKVERSION}.sdk/usr/lib/libz.dylib \
${DEVELOPER}/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS${SDKVERSION}.sdk/usr/lib/libz.dylib \
-output ${OUTPUTDIR}/lib/libz.a

cp -R ${DEVELOPER}/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator${SDKVERSION}.sdk/usr/include/zlib* ${OUTPUTDIR}/include/

#Salvo la variabile d'ambiente PATH orginale per poi aggiungere quelle dell sdk di xcode 
export ORIGINALPATH=$PATH


for ARCH in ${ARCHS}
do
	if [ "${ARCH}" == "i386" ] || [ "${ARCH}" == "x86_64" ]; then
		PLATFORM="iPhoneSimulator"
		EXTRA_CONFIG=""
	else
		PLATFORM="iPhoneOS"
		EXTRA_CONFIG="--disable-tool-name-check --host=arm-apple-darwin14 --target=arm-apple-darwin14 --disable-gcc-hardening --disable-linker-hardening"
	fi
	
	#Creo le cartelle per le varie archietetutre e piattaforme
	mkdir -p "${INTERDIR}/${PLATFORM}${SDKVERSION}-${ARCH}.sdk"
	mkdir -p "${INTERDIR}/${PLATFORM}${SDKVERSION}-${ARCH}.sdk/include"
	mkdir -p "${INTERDIR}/${PLATFORM}${SDKVERSION}-${ARCH}.sdk/lib"

	export PATH="${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/:${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/usr/bin/:${DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin:${DEVELOPER}/usr/bin:${ORIGINALPATH}"

	export CC="/usr/bin/gcc -arch ${ARCH} -miphoneos-version-min=${MINIOSVERSION}"
	export CXX="/usr/bin/g++ -arch ${ARCH} -miphoneos-version-min=${MINIOSVERSION}"
	
	echo "${INTERDIR}/${PLATFORM}${SDKVERSION}-${ARCH}.sdk"

	./configure ${EXTRA_CONFIG} \
	--prefix="${INTERDIR}/${PLATFORM}${SDKVERSION}-${ARCH}.sdk" \
    	--with-libevent="${REPOROOT}/../iOS-OnionBrowser/dependencies" \
    	LDFLAGS="$LDFLAGS -L${OUTPUTDIR}/lib" \
	CFLAGS="$CFLAGS -O2 -I${OUTPUTDIR}/include -isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk" \
	CPPFLAGS="$CPPFLAGS -I${OUTPUTDIR}/include -isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk" \
    	CXXFLAGS="$CXXFLAGS -I${OUTPUTDIR}/include -isysroot ${DEVELOPER}/Platforms/${PLATFORM}.platform/Developer/SDKs/${PLATFORM}${SDKVERSION}.sdk"
	
	make -j4
	make install
	make clean	
done




OUTPUT_LIB="libight.a"
INPUT_LIBS=""

for ARCH in ${ARCHS}; do
	if [ "${ARCH}" == "i386" ] || [ "${ARCH}" == "x86_64" ];
	then
		PLATFORM="iPhoneSimulator"
	else
		PLATFORM="iPhoneOS"
	fi
	INPUT_ARCH_LIB="${INTERDIR}/${PLATFORM}${SDKVERSION}-${ARCH}.sdk/lib/${OUTPUT_LIB}"
	if [ -e $INPUT_ARCH_LIB ]; then
		INPUT_LIBS="${INPUT_LIBS} ${INPUT_ARCH_LIB}"
	fi
	# Combine the three architectures into a universal library.
	if [ -n "$INPUT_LIBS" ]; then
		lipo -create $INPUT_LIBS \
		-output "${OUTPUTDIR}/lib/${OUTPUT_LIB}"
	else
		echo "$OUTPUT_LIB does not exist, skipping (are the dependencies installed?)"
	fi
done



#Cerchiamo gli header e li spostiamo in build/built/.../include
#find src/common -name "*.hpp" -exec cp {} "${OUTPUTDIR}/include/common" \;
#find src/ext -name "*.hpp" -exec cp {} "${OUTPUTDIR}/include/ext" \;
#find src/net -name "*.hpp" -exec cp {} "${OUTPUTDIR}/include/net" \;
#find src/ooni -name "*.hpp" -exec cp {} "${OUTPUTDIR}/include/ooni" \;
#find src/protocols -name "*.hpp" -exec cp {} "${OUTPUTDIR}/include/protocols" \;
#find src/report -name "*.hpp" -exec cp {} "${OUTPUTDIR}/include/report" \;
#find src/common -name "*.h" -exec cp {} "${OUTPUTDIR}/include/common"\ ;
#find src/ext -name "*.h" -exec cp {} "${OUTPUTDIR}/include/ext" \;
#find src/net -name "*.h" -exec cp {} "${OUTPUTDIR}/include/net" \;
#find src/oon -name "*.h" -exec cp {} "${OUTPUTDIR}/include/ooni" \;
#find src/protocols -name "*.h" -exec cp {} "${OUTPUTDIR}/include/protocols" \;
#find src/report -name "*.h" -exec cp {} "${OUTPUTDIR}/include/report" \;
echo "Building done."
echo "Cleaning up..."


echo "Done."
